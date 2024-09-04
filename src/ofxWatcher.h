#pragma once

#include "ofFileUtils.h"
#include <map>
#include <regex>

struct ofEventArgs;

namespace ofx {
namespace watcher {

class Token
{
public:
	Token(std::string str);
	bool isMatch(std::string name) const;
	bool isRecursive() const { return is_recursive_; }
private:
	std::regex regex_;
	bool is_recursive_=false;
};

enum FileTypeFlag {
	NONE		= 1 << 0,
	NOT_FOUND	= 1 << 1,
	REGULAR		= 1 << 2,
	DIRECTORY	= 1 << 3,
	// the following may not apply to some operating systems or file systems
	SYMLINK		= 1 << 4,
	BLOCK		= 1 << 5,
	CHARACTER	= 1 << 6,
	FIFO		= 1 << 7,
	SOCKET		= 1 << 8,
	REPARSE		= 1 << 9,  // Windows: FILE_ATTRIBUTE_REPARSE_POINT that is not a symlink
	UNKNOWN		= 1 << 10,
	
	DEFAULT = REGULAR | DIRECTORY,
};

class PathFinder
{
public:
	struct Option {
		std::vector<std::string> excludes={".DS_Store"};
		std::vector<std::string> allow_ext={};
		bool filter(std::filesystem::path path) const;
	};

	PathFinder(std::string path);
	std::vector<std::filesystem::path> getFileList(Option option, FileTypeFlag type_flag=DEFAULT) const;

	void update(Option option, FileTypeFlag type_flag);
	const std::vector<std::filesystem::path>& getUpdated() const { return updated_; }
private:
	std::filesystem::path base_;
	std::vector<Token> token_;
	std::vector<std::filesystem::path> listDir(std::filesystem::path path, bool recursive, Option option) const;
	
	std::map<std::filesystem::path, std::filesystem::file_time_type> files_cache_;
	std::vector<std::filesystem::path> updated_;
	bool update(std::filesystem::path path);
};

class Watcher
{
public:
	struct Option {
		float check_interval = 1;
		std::size_t file_type_flag = REGULAR;
		bool absolute_path = false;
		PathFinder::Option finder_option = {};
	};
	Watcher(std::filesystem::path path, std::function<void(std::filesystem::path)> func);
	virtual ~Watcher() { stop(); }
	void setOption(Option option) { option_ = option; }
	void check();
	void start();
	void stop();
private:
	void check(ofEventArgs&);
	void check(float delta_time);
	PathFinder finder_;
	std::function<void(std::filesystem::path)> func_;
	bool is_watch_=false;
	float timer_=0;
	Option option_;
};
}}

typedef ofx::watcher::Watcher ofxWatcher;
typedef ofxWatcher::Option ofxWatcherOption;

namespace detail {

template<typename Func, typename... Args>
auto apply(Func f, Args &&...args)
-> decltype(f(args...)) {
	return f(std::forward<Args>(args)...);
}

template<typename Func, typename Arg, typename ...Rest>
auto apply(Func f, Arg &&arg, Rest &&...rest)
-> decltype(f(arg)) {
	return f(std::forward<Arg>(arg));
}

template<typename Func, typename... Args>
auto apply(Func f, Args &&...args)
-> decltype(f()) {
	return f();
}
}

namespace {
std::multimap<std::filesystem::path, std::shared_ptr<ofxWatcher>> watchers;
}

template<typename Func>
std::shared_ptr<ofxWatcher> ofxWatchPath(std::filesystem::path path, Func func, ofxWatcherOption option={}, bool retain=true)
{
	auto subscriber = std::make_shared<ofxWatcher>(path, [func](std::filesystem::path path) {
		detail::apply(func, path, path);
	});
	if(retain) {
		watchers.insert({path, subscriber});
	}
	subscriber->setOption(option);
	subscriber->start();
	return subscriber;
}

template<typename Loader, typename Then>
std::shared_ptr<ofxWatcher> ofxWatchPath(std::filesystem::path path, Loader loader, Then then, ofxWatcherOption option={}, bool retain=true)
{
	return ofxWatchPath(path, [loader,then](std::filesystem::path path) {
		detail::apply(then, loader(path), path);
	}, option, retain);
}

static inline bool ofxUnwatchPath(std::filesystem::path path)
{
	auto range = watchers.equal_range(path);
	if(range.first == end(watchers)) {
		return false;
	}
	watchers.erase(range.first, range.second);
	return true;
}
