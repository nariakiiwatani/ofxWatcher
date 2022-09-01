#include "ofxWatcher.h"
#include "ofConstants.h"
#include "ofEvents.h"
#include "ofUtils.h"
#include "ofAppRunner.h"

using namespace ofx::watcher;
namespace fs = std::filesystem;

Token::Token(std::string str)
{
	if(str == "**") {
		is_recursive_ = true;
		str = "*";
	}
	const std::regex esc(R"([.^$|()\[\]{}+?\\])");	// remove * from regular list
	std::string escaped_str = std::regex_replace(str, esc, R"(\\&)",
									   std::regex_constants::match_default | std::regex_constants::format_sed);
	regex_ = std::regex_replace("^"+escaped_str+"$", std::regex(R"(\*)"), ".*");
}

bool Token::isMatch(std::string name) const
{
	return std::regex_match(name, regex_);
}


namespace {
std::size_t get_type_flag(fs::file_type type) {
	switch(type) {
		case fs::status_error:		return ::NONE;
		case fs::file_not_found:	return ::NOT_FOUND;
		case fs::regular_file:		return ::REGULAR;
		case fs::directory_file:	return ::DIRECTORY;
		case fs::symlink_file:		return ::SYMLINK;
		case fs::block_file:		return ::BLOCK;
		case fs::character_file:	return ::CHARACTER;
		case fs::fifo_file:			return ::FIFO;
		case fs::socket_file:		return ::SOCKET;
		case fs::reparse_file:		return ::REPARSE;
		case fs::type_unknown:		return ::UNKNOWN;
		default: return 0;
	}
}
template<typename Iter>
std::vector<fs::path> listDir(Iter it, Iter end_it, PathFinder::Option option) {
	std::vector<fs::path> ret;
	while(it != end_it) {
		fs::path path = *it++;
		if(option.filter(path)) {
			ret.push_back(path);
		}
	}
	return ret;
}
}

PathFinder::PathFinder(std::string path)
{
	auto wildcard_pos = path.find('*');
	if(wildcard_pos == std::string::npos) {
		base_ = path;
		return;
	}
	auto slash_pos = path.rfind('/', wildcard_pos);
	auto before_slash = slash_pos != std::string::npos ? path.substr(0, slash_pos) : "";
	auto after_slash = slash_pos != std::string::npos ? path.substr(slash_pos) : path;
	base_ = before_slash;
	token_ = [](std::string path) {
		std::vector<Token> ret;
		char *ptr;  
		ptr = strtok(const_cast<char*>(path.data()), "/");
		while(ptr != nullptr) {
			ret.emplace_back(ptr);
			ptr = strtok(nullptr, "/");  
		}  
		return ret;
	}(after_slash);
}

std::vector<fs::path> PathFinder::getFileList(Option option, FileTypeFlag file_type_flag) const
{
	auto ret = listDir(base_, false, option);
	for(int i = 0; i < token_.size(); ++i) {
		bool is_last_token = i == token_.size()-1;
		auto &token = token_[i];
		ret.erase(remove_if(begin(ret), end(ret), [&](fs::path path) {
			auto filename = path.filename();
			bool is_file = fs::exists(path) && !fs::is_directory(path);
			if(is_file && !is_last_token) {
				return true;
			}
			return !token.isMatch(filename.string());
		}), end(ret));
		if(!is_last_token || token.isRecursive()) {
			decltype(ret) new_list;
			for(auto &&dir : ret) {
				auto append = listDir(dir, token.isRecursive(), option);
				std::move(begin(append), end(append), std::back_inserter(new_list));
			}
			std::swap(ret, new_list);
		}
	}
	ret.erase(remove_if(begin(ret), end(ret), [file_type_flag](fs::path path) {
		auto file_type = get_type_flag(fs::status(path).type());
		return (file_type & file_type_flag) == 0;
	}), end(ret));
	return ret;
}

bool PathFinder::Option::filter(fs::path path) const
{
	auto filename = path.filename();
	if(find(begin(excludes), end(excludes), filename) != end(excludes)) {
		return false;
	}
	if(!allow_ext.empty()) {
		auto ext = filename.extension();
		if(ext.empty()) {
			if(!fs::is_directory(path)) {
				return false;
			}
		}
		else {
			if(find(begin(allow_ext), end(allow_ext), ext) == end(allow_ext)) {
				return false;
			}
		}
	}
	return true;
}

std::vector<fs::path> PathFinder::listDir(fs::path path, bool recursive, Option option) const
{
	if(!fs::exists(path)) {
		return {};
	}
	if(!fs::is_directory(path)) {
		return {path};
	}
	if(recursive) {
		return ::listDir(fs::recursive_directory_iterator(path), {}, option);
	}
	else {
		return ::listDir(fs::directory_iterator(path), {}, option);
	}
}

void PathFinder::update(Option option, FileTypeFlag type_flag) {
	updated_.clear();
	auto list = getFileList(option, type_flag);
	for(auto &&path : list) {
		if(update(path)) {
			updated_.push_back(path);
		}
	}
}

bool PathFinder::update(fs::path path) {
	auto last_write_time = fs::last_write_time(ofToDataPath(path));
	auto result = files_cache_.insert({path, last_write_time});
	if(result.second || result.first->second != last_write_time) {
		result.first->second = last_write_time;
		return true;
	}
	return false;
}

Watcher::Watcher(fs::path path, std::function<void(fs::path)> func)
:finder_(ofToDataPath(path.string(), true)), func_(func)
{
}

void Watcher::check(ofEventArgs&)
{
	check(ofGetLastFrameTime());
}

void Watcher::check(float delta_time)
{
	timer_ += delta_time;
	if(timer_ >= option_.check_interval) {
		timer_ -= option_.check_interval;
		check();
	}
}

void Watcher::check()
{
	finder_.update(option_.finder_option, (FileTypeFlag)option_.file_type_flag);
	auto updated = finder_.getUpdated();
	for(auto path : updated) {
		if(!option_.absolute_path) {
			path = ofFilePath::makeRelative(ofToDataPath("",true), path);
		}
		func_(path);
	}
}

void Watcher::start() {
	if(!is_watch_) {
		is_watch_ = true;
		timer_ = 0;
		check();
		ofAddListener(ofEvents().update, this, &Watcher::check);
	}
}

void Watcher::stop() {
	if(is_watch_) {
		is_watch_ = false;
		ofRemoveListener(ofEvents().update, this, &Watcher::check);
	}
}

