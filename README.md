# ofxWatcher

Notify when changes are made to a file or directory.

# usage

`ofxWatchPath(path, callback, [option], [retain])`  
`ofxWatchPath(path, loader, callback, [option], [retain])`

## path

A path is specified as a relative or absolute path from the data folder.  
It can be a file or a directory or a path with wildcards.  
A wildcard is `*`(matches to any string) or `**`(matches to any directory recursively).  
If the path is for a directory, it watches all of the children.

(Example)  
If there are these files...
1. `"videos/highQ/recorded.mov"`
1. `"videos/highQ/recorded.mp4"`
1. `"videos/lowQ/half.mov"`
1. `"videos/lowQ/sequence.mov"`
1. `"settings/playback/sequence.json"`

- `"videos/*/*.mov"` matches to `1,3,4`
- `"**/highQ"` matches to `1,2`
- `"**/sequence*"` matches to `4,5`
- `"**"` matches to `1,2,3,4,5`

## callback

When changes are made to a file or directory while watching, the callback is called.  
Callback functions can take the following 4 forms.  
You can use function pointer or lambda as well.

1. `callback(void)`
1. `callback(const std::filesystem::path&)`
1. `callback(const MediumType&)`
1. `callback(const MediumType&, const std::filesystem::path&)`

`MediumType` is a product of loader function.

## loader

Loader function modifies a path into any types.  
It is mainly intended to be used for loading the file.

(Example)

```C++
ofxWatchPath("settings/*.json", ofLoadJson, [](const ofJson &json) {
	std::cout << json.dump(2) << std::endl;
});
```

__Tips__  
If you want to use an existing function that expects arguments other than filepath or a member function, consider using `std::bind`.

## option

```C++
ofxWatcherOption option;
option.check_interval = 10;
option.file_type_flag = ofx::watcher::REGULAR | ofx::watcher::DIRECTORY;
option.absolute_path = true;
option.finder_option.excludes = {".DS_Store",".git"};
option.finder_option.allow_ext = {".png",".jpg"};
```

### float check_interval

Check for modifies at this interval in seconds.  
default: `1`

### std::size_t file_type_flag

Only checks files or directories of this type.  
See `FileTypeFlag` in `ofxWatcher.h`  
default: `REGULAR`

### bool absolute_path

If true, callback or loader is called with absolute path.  
default: `false`

### std::vector<std::string> excludes

Excludes files or directories that has a name in this vector.  
An exact match is required.  
default: `[".DS_Store"]`

### std::vector<std::string> allow_ext

If specified, files with unspecified extensions are ignored.  
Each ext should start with `.`.  
default: `(empty)`

### bool retain

If true, ofxWatcher keeps the `shared_ptr` created by `ofxWatchPath`.  
If you want to control the object's lifecycle, call `ofxWatchPath` with `retain=false`.  
default: `true`