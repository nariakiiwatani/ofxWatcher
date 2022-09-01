#include "ofApp.h"
#include "ofxWatcher.h"

//--------------------------------------------------------------
void ofApp::setup(){
	using namespace std;
	namespace fs = std::filesystem;
	
	// -------- basic usage --------
	{
		cout << "1: watch all files in data/images" << endl;
		// ex) images/face.jpg, images/log, images/fix.png,,,
		ofxWatchPath("images", [](const fs::path &str) {
			cout << "1: " << str << endl;
		});
	}
	{
		cout << "2: wildcard jpg" << endl;
		// ex) images/face.jpg, images/hand.jpg, images/tracked.jpg,,,
		ofxWatchPath("images/*.jpg", [](const fs::path &str) {
			cout << "2: " << str << endl;
		});
	}
	{
		cout << "3: use option to filter by file extension" << endl;
		// ex) images/face.jpg, images/hand.jpg, images/tracked.jpg,,,
		ofxWatcherOption opt;
		opt.finder_option.allow_ext.push_back(".jpg");
		ofxWatchPath("images", [](const fs::path &str) {
			cout << "3: " << str << endl;
		}, opt);
	}

	// -------- recursive wildcard --------
	{
		cout << "4: recursive search by **" << endl;
		// ex) images/face.jpg, images/log/error.log, images/fix.png,,,
		ofxWatchPath("images/**", [](const fs::path &str) {
			cout << "4: " << str << endl;
		});
	}
	{
		cout << "5: specify something after recursive search" << endl;
		// ex) images/tracking.mp4, videos/animated.mp4, backup/videos/result.mp4,,,
		ofxWatchPath("**/*.mp4", [](const fs::path &str) {
			cout << "5: " << str << endl;
		});
	}
	
	// -------- loader --------
	{
		cout << "6: use custom loader" << endl;
		ofxWatchPath("json/*.json", ofLoadJson, [](const ofJson &json, const fs::path &path) {
			cout << "6: " << path << ":" << json.dump(2) << endl;
		});
	}

	// -------- advanced --------
	{
		cout << "7: filter by file_type" << endl;
		// ex) images/face.jpg, images/log, images/fix.png,,,
		using namespace ofx::watcher;
		ofxWatcherOption opt;
		opt.file_type_flag = DIRECTORY | REGULAR;	// to match directories and files
		ofxWatchPath("**", [](const fs::path &str) {
			cout << "7: " << str << endl;
		}, opt);
	}
	{
		cout << "8: callback with no argument" << endl;
		ofxWatchPath("json/*.json", []() {
			cout << "8: file updated" << endl;
		});
	}
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
