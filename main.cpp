#include <fstream>
#include "jsoncpp/include/json/json.h"
#include "curl/curl.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <vector>
#include <opencv2/opencv.hpp>
#include <thread>
#include <chrono> 
#include <array>
#ifdef _WIN32
#include <windows.h>
#endif


#define CV_8UC3

enum axis {height, width};
enum searchType {pools, tags};
const int DisplayHeight = 1080;
const int DisplayWidth = 1920;
const int maxWindows = 5;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

inline bool doesConfigExist() {
  return std::filesystem::exists("config.json");
}

void createConfig() {
  Json::StyledWriter writer;
  Json::Value startingConfig;
  startingConfig["login"] = "Write Username";
  startingConfig["api_key"] = "Find API key from E621 and paste";
  startingConfig.setComment(static_cast<Json::String>("//Set tags (example: 'fluffy+female+-biped' would be for fluffy female -biped on E621)"), Json::commentAfterOnSameLine);
  startingConfig["tags"] = "fluffy+female+-biped";
  startingConfig["pools[ids]"] = "";
  startingConfig.setComment(static_cast<Json::String>("//For pools you can add pool ids (found in the html link), seperated by commas for multiple"), Json::commentAfterOnSameLine);
  startingConfig["min_seconds_between_images"] = 5;
  startingConfig["max_seconds_between_images"] = 30;
  startingConfig["min_seconds_of_image"] = 5;
  startingConfig["max_seconds_of_image"] = 30;

  std::string configString = startingConfig.toStyledString();


  std::filesystem::path path{ "./" };
  path /= "config.json";
  std::filesystem::create_directories(path.parent_path()); //add directories based on the object path (without this line it will not work)

  std::ofstream ofs(path);
  ofs << configString; 
  ofs.close();
}

int getRandomPost(Json::Value posts){
  int size = posts["posts"].size();
  [[assume]](size == 100);
  std::random_device rd;
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(0, size-1);
  return static_cast<int>(distrib(gen));
}

int getRandomPageId(size_t pagenum, std::vector<int> startIds){
  std::random_device rd;
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(0, pagenum-1);
  return startIds[distrib(gen)];
}

std::pair<int, int> getRandomWindowSize(int limitingSize, axis limitingSide){
  std::random_device rd;
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  
  
  if (limitingSize == height) {
    std::uniform_int_distribution<> distrib(static_cast<int>(0.1 * DisplayHeight), static_cast<int>(0.3 * DisplayHeight));
    int height = distrib(gen);
    int width = static_cast<int>(16.0/9.0 * height);
    return {height, width};
  } else {
    std::uniform_int_distribution<> distrib(static_cast<int>(0.1 * DisplayWidth), static_cast<int>(0.3 * DisplayWidth));
    int width = distrib(gen);
    int height = static_cast<int>(9.0/16.0 * width);
    return {height, width};
  }
}

std::pair<int, int> getRandomWindowPosition(int windowWidth, int windowHeigh) {
  std::random_device rd;
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distribW(1, DisplayWidth - windowWidth);
  int movedX = distribW(gen);
  std::uniform_int_distribution<> distribH(1, DisplayHeight - windowHeigh);
  int movedY = distribH(gen);
  return {movedX, movedY};
}

void randomSleepTime(int min, int max){
  std::random_device rd;
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(min, max);
  int selectedTime = distrib(gen);
  std::this_thread::sleep_for (std::chrono::seconds(selectedTime));
}

std::string getRandomPostId(std::vector<std::string>& postIds){
  std::random_device rd;
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(0, postIds.size());
  int selectedPost = distrib(gen);
  return postIds[selectedPost];
}

std::pair<std::string,std::string> getPictureLink(std::string login, std::string key, std::string tags,  size_t& pagenum, std::vector<int>& startId, bool& lastPageFount){
  CURL *curl;
  CURLU *curlu = curl_url();
  CURLUcode rc;
  CURLcode res;
  std::string readBuffer;
  curl = curl_easy_init();
  struct curl_slist *headers = NULL; 
  headers = curl_slist_append(headers, "User-Agent: hornyVirus/0.1 (by ProgrammingFurry on e621)");
  
  login = "login="+login;
  key = "api_key="+key;
  tags = "tags="+tags;

  rc = curl_url_set(curlu, CURLUPART_URL, "https://e621.net/posts.json", 0);
  rc = curl_url_set(curlu, CURLUPART_HOST, "e621.net", 0);
  rc = curl_url_set(curlu, CURLUPART_PATH, "posts.json", 0);
  rc = curl_url_set(curlu, CURLUPART_SCHEME, "https", 0);
  rc = curl_url_set(curlu, CURLUPART_QUERY, login.c_str(), 0);
  rc = curl_url_set(curlu, CURLUPART_QUERY, key.c_str(), CURLU_APPENDQUERY);
  
  rc = curl_url_set(curlu, CURLUPART_QUERY, "limit=100", CURLU_APPENDQUERY);
  rc = curl_url_set(curlu, CURLUPART_QUERY, tags.c_str(), CURLU_APPENDQUERY);

  if (lastPageFount) {
    std::string pageQuery = "page=a";
    int startingId = getRandomPageId(pagenum, startId);
    rc = curl_url_set(curlu, CURLUPART_QUERY, (pageQuery+std::to_string(startingId)).c_str(), CURLU_APPENDQUERY);
  } else {
    int id = startId[pagenum-1];
    std::string pageQuery = "page=a"+ std::to_string(id);
    rc = curl_url_set(curlu, CURLUPART_QUERY, pageQuery.c_str(), CURLU_APPENDQUERY);
  }
  
  char *url;
  rc = curl_url_get(curlu, CURLUPART_URL, &url, 0);
  std::cout << url << std::endl;

  res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  res = curl_easy_setopt(curl, CURLOPT_URL, url);
  res = curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  res = curl_easy_setopt(curl, CURLOPT_HTTPGET, true);
  res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  res = curl_easy_perform(curl);
  Json::Value obj;
  Json::Reader reader;
  reader.parse(readBuffer, obj); 

  if (pagenum > 1 && obj["posts"].size() <= 1) {
    pagenum -=1;
    lastPageFount = true;
  }
  
  if (res == CURLE_HTTP_RETURNED_ERROR ) {
    std::cout << "Request failed. Check if all settings in config are correct and that there exist posts with these tags" << std::endl;
  }
  curl_easy_cleanup(curl);
  curl_free(url);
  curl_url_cleanup(curlu);

  
  
  if (obj["posts"].size() == 100 && !lastPageFount) {
    std::cout <<"Next page";
    pagenum +=1 ;
    startId.push_back(obj["posts"][99]["id"].asInt());
    std::cout << std::to_string(startId[pagenum-1]);
  } else {lastPageFount = true;}

  int randInt = getRandomPost(obj);
  std::cout << obj["posts"].size() << std::endl;
  return {obj["posts"][randInt]["file"]["url"].asString(), obj["posts"][randInt]["id"].asString()};
}

std::pair<std::string,std::string> getPictureLink(std::string login, std::string key, std::string pools){
  CURL *curl;
  CURLU *curlu = curl_url();
  CURLUcode rc;
  CURLcode res;
  std::string readBuffer;
  curl = curl_easy_init();
  struct curl_slist *headers = NULL; 
  headers = curl_slist_append(headers, "User-Agent: hornyVirus/0.1 (by ProgrammingFurry on e621)");
  
  login = "login="+login;
  key = "api_key="+key;
  pools = "page=a"+pools;

  rc = curl_url_set(curlu, CURLUPART_URL, "https://e621.net/posts.json", 0);
  rc = curl_url_set(curlu, CURLUPART_HOST, "e621.net", 0);
  rc = curl_url_set(curlu, CURLUPART_PATH, "posts.json", 0);
  rc = curl_url_set(curlu, CURLUPART_SCHEME, "https", 0);
  rc = curl_url_set(curlu, CURLUPART_QUERY, login.c_str(), 0);
  rc = curl_url_set(curlu, CURLUPART_QUERY, key.c_str(), CURLU_APPENDQUERY);
  rc = curl_url_set(curlu, CURLUPART_QUERY, "limit=1", CURLU_APPENDQUERY);
  rc = curl_url_set(curlu, CURLUPART_QUERY, (pools).c_str(), CURLU_APPENDQUERY);
  
  char *url;
  rc = curl_url_get(curlu, CURLUPART_URL, &url, 0);
  std::cout << url << std::endl;

  res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  res = curl_easy_setopt(curl, CURLOPT_URL, url);
  res = curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  res = curl_easy_setopt(curl, CURLOPT_HTTPGET, true);
  res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  res = curl_easy_perform(curl);
  Json::Value obj;
  Json::Reader reader;
  reader.parse(readBuffer, obj); 
    
    curl_easy_cleanup(curl);
    curl_free(url);
    curl_url_cleanup(curlu);

    return {obj["posts"][0]["file"]["url"].asString(), obj["posts"][0]["id"].asString()};
}

std::vector<std::string> getIdsFromPools(std::string login, std::string key, std::string pools){
  CURL *curl;
  CURLU *curlu = curl_url();
  CURLUcode rc;
  CURLcode res;
  std::string readBuffer;
  curl = curl_easy_init();
  struct curl_slist *headers = NULL; 
  headers = curl_slist_append(headers, "User-Agent: hornyVirus/0.1 (by ProgrammingFurry on e621)");
  
  login = "login="+login;
  key = "api_key="+key;
  pools = "search[id]="+pools;

  rc = curl_url_set(curlu, CURLUPART_URL, "https://e621.net/pools?format=json", 0);
  rc = curl_url_set(curlu, CURLUPART_SCHEME, "https", 0);
  rc = curl_url_set(curlu, CURLUPART_QUERY, login.c_str(), CURLU_APPENDQUERY);
  rc = curl_url_set(curlu, CURLUPART_QUERY, key.c_str(), CURLU_APPENDQUERY);
  rc = curl_url_set(curlu, CURLUPART_QUERY, pools.c_str(), CURLU_APPENDQUERY);

  char *url;
  rc = curl_url_get(curlu, CURLUPART_URL, &url, 0);
  std::cout << url << std::endl;

  res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  res = curl_easy_setopt(curl, CURLOPT_URL, url);
  res = curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  res = curl_easy_setopt(curl, CURLOPT_HTTPGET, true);
  res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  res = curl_easy_perform(curl);
  Json::Value obj;
  Json::Reader reader;
  reader.parse(readBuffer, obj); 

  std::vector<std::string> postIds;

  for (auto elem : obj) {
    for (auto elem2 : elem["post_ids"]){
      postIds.push_back(elem2.asString());
    }
  }

  
  if (res == CURLE_HTTP_RETURNED_ERROR ) {
    std::cout << "Request failed. Check if all settings in config are correct" << std::endl;
  }
  curl_easy_cleanup(curl);
  curl_free(url);
  curl_url_cleanup(curlu);
  return postIds;
}

std::string displayImage(const std::string & imagePath, const Json::Value & settings, std::string Id) {
  CURL* curl_handle = curl_easy_init();
  CURLcode res;

  std::string buffer;

  struct curl_slist *headers = NULL; 
  headers = curl_slist_append(headers, "User-Agent: hornyVirus/0.1 (by Nyara on e621)");
	
	curl_easy_setopt(curl_handle, CURLOPT_URL, (imagePath+"?login="+settings["login"].asString()+"&api_key="+settings["api_key"].asString()).c_str());

  curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &buffer);

  
	res = curl_easy_perform(curl_handle);
  
	/* check for errors */
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
	} else {
		

		//printf("%lu bytes retrieved\n", (long) chunk.size);

		std::vector<uint8_t> vec(buffer.begin(), buffer.end());

    //cv::Mat temp(1, chunk.size, CV_8UC3(3),  chunk.memory);
		cv::Mat image = cv::imdecode(vec, cv::IMREAD_UNCHANGED);
		cv::namedWindow(Id, cv::WINDOW_NORMAL + cv::WINDOW_GUI_NORMAL);

    //Give window a random size not exceeding a quarter of the screen
    axis largerSize = width;
    cv::Rect imageSize= cv::getWindowImageRect(Id);
    int sizeOfAxis = imageSize.width;
    if (imageSize.height > (imageSize.width * 16/9)){ //height is more than width on generic 16/9 monitor
      largerSize = height;
      sizeOfAxis = imageSize.height;
    }
      auto [newHeight, newWidth] = getRandomWindowSize(sizeOfAxis, largerSize);

      cv::resizeWindow(Id, newWidth, newHeight);
      cv::setWindowTitle(Id, "Gay :3");
      auto [movedX, movedY] = getRandomWindowPosition(newWidth, newHeight);
      cv::setWindowProperty(Id, cv::WND_PROP_TOPMOST, 1);

		if (!image.empty()) {
      cv::startWindowThread	();
			cv::imshow(Id, image);
      cv::moveWindow(Id, movedX, movedY);
      cv::setWindowProperty(Id, cv::WND_PROP_AUTOSIZE, cv::WINDOW_AUTOSIZE );
      std::cout << std::endl << "New width and height" << newWidth << ", " << newHeight << std::endl;
      std::cout << std::endl << "New position" << movedX << ", " << movedY << std::endl;
      
	    curl_easy_cleanup(curl_handle);
      return (Id);

		}
	}

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

  return("No Id");
}

void destroyWindow(std::string threadWindow, int timeBeforeMin, int timeAfterMin) {
  randomSleepTime(timeBeforeMin, timeAfterMin);
  cv::destroyWindow(threadWindow);
}

void windowThreadTags(Json::Value settings, size_t& pagenum, std::vector<int>& startIds, bool& lastPageFound, size_t & numOfWindows, int min_seconds_of_image, int max_seconds_of_image, std::array<bool, maxWindows>& isAvailable, int IdOfThread) {
  isAvailable[IdOfThread] = false;
  auto [imagePath, Id] = getPictureLink(settings["login"].asString(), settings["api_key"].asString(), settings["tags"].asString(), pagenum, startIds, lastPageFound);

  std::string threadWindow = displayImage(imagePath, settings, Id);

  #ifdef _WIN32
  windows::BringWindowToTop(threadWindow);
  #endif

  const std::string error = "No Id";
  if (threadWindow == error) {numOfWindows--; isAvailable[IdOfThread] = true; return;} else {destroyWindow(threadWindow, min_seconds_of_image, max_seconds_of_image);};
  numOfWindows--;
  isAvailable[IdOfThread] = true;
  return;
}

void windowThreadPools(Json::Value settings, size_t & numOfWindows, int min_seconds_of_image, int max_seconds_of_image, std::array<bool, maxWindows>& isAvailable, int IdOfThread, std::vector<std::string> postIds) {
  isAvailable[IdOfThread] = false;
  
  std::string randomPostId = getRandomPostId(postIds);

  auto [imagePath, Id] = getPictureLink(settings["login"].asString(), settings["api_key"].asString(), randomPostId);
  
  std::string threadWindow = displayImage(imagePath, settings, Id);

  #ifdef _WIN32
  windows::BringWindowToTop(threadWindow);
  #endif

  const std::string error = "No Id";
  if (threadWindow == error) {numOfWindows--; isAvailable[IdOfThread] = true; return;} else {destroyWindow(threadWindow, min_seconds_of_image, max_seconds_of_image);};
  numOfWindows--;
  isAvailable[IdOfThread] = true;
  return;
}

int findClear(bool & isClear, std::array<bool, maxWindows>& isAvailable, int maxWindows){
  for (int i=0; i<maxWindows; i++){
    if (isAvailable[i] == true) {
      isClear = true;
      return i;
    }
  }
  return maxWindows+1;
}

int main() {
  if (!doesConfigExist()) {
    createConfig();
  }

  bool lastPageFound = false;
  size_t pagenum = 1;
  std::vector<int> startIds = {1};

  Json::Value settings; //creates a JSON object from config file
  std::ifstream configString("./config.json");
  Json::Reader reader;
  reader.parse(configString, settings);
  configString.close();

  size_t numOfWindows = 0;

  for (char elem : settings["api_key"].asString()) {
    if (std::isspace(static_cast<unsigned char>(elem))) {
      throw std::invalid_argument("Api key is not written in config.json or is written incorrectly. Make sure there are no spaces anywhere in the API key.");
    }
  }

  char selection;

  std::cout<< std::endl << "If you wish to use the pools selected in the config press 'p', if you wish to use the tags, press 't', afterwards press enter" << std::endl;
  label:
  std::cin >> selection;
  selection = tolower(selection);
  searchType selectedType;

  if (selection == 'p') {
    selectedType = pools;
  } else if (selection == 't') {
    selectedType = tags;
  } else{
    std::cout <<"Unknown selection, please select 'p' for pools or 't' for tags" << std::endl;
    goto label;
  }

  int min_seconds_between_images = settings["min_seconds_between_images"].asInt();
  int max_seconds_between_images = settings["max_seconds_between_images"].asInt();
  int min_seconds_of_image = settings["min_seconds_of_image"].asInt();
  int max_seconds_of_image = settings["max_seconds_of_image"].asInt();


  if (min_seconds_between_images < 2) {min_seconds_between_images = 2;};
  if (max_seconds_between_images < 2) {max_seconds_between_images = 2;};
  if (max_seconds_between_images < min_seconds_between_images) {max_seconds_between_images, min_seconds_between_images = min_seconds_between_images, max_seconds_between_images;};
  if (max_seconds_of_image < min_seconds_of_image) {max_seconds_of_image, min_seconds_of_image = min_seconds_of_image, max_seconds_of_image;};
  
  std::array<std::thread, maxWindows> threads;
  bool isClear = false;
  std::array<bool, maxWindows> isAvailable;
  std::fill(isAvailable.begin(),isAvailable.end(), true);

  if (selectedType == tags){
  while(true) {
    curl_global_init(CURL_GLOBAL_ALL);
    isClear = false;
    int clearSpot = 0;
    
    clearSpot = findClear(isClear, isAvailable, maxWindows);
    if (isClear){
     if (threads[clearSpot].joinable()){threads[clearSpot].join();}
    threads[clearSpot] = std::thread(windowThreadTags, settings, std::ref(pagenum), std::ref(startIds), std::ref(lastPageFound), std::ref(numOfWindows), min_seconds_of_image, max_seconds_of_image, std::ref(isAvailable), clearSpot);
    
    numOfWindows++;} else {
      if (threads[2].joinable()){threads[2].join();}
      
    }

    randomSleepTime(min_seconds_between_images, max_seconds_between_images);
    
    curl_global_cleanup();
  }}
  else if (selectedType == pools){
    std::vector postIds = getIdsFromPools(settings["login"].asString(), settings["api_key"].asString(), settings["pools[ids]"].asString());
    while(true){
      curl_global_init(CURL_GLOBAL_ALL);
      isClear = false;
      int clearSpot = 0;
      
      clearSpot = findClear(isClear, isAvailable, maxWindows);
      if (isClear){
      if (threads[clearSpot].joinable()){threads[clearSpot].join();}
      threads[clearSpot] = std::thread(windowThreadPools, settings, std::ref(numOfWindows), min_seconds_of_image, max_seconds_of_image, std::ref(isAvailable), clearSpot, postIds);
      
      numOfWindows++;} else {
        if (threads[2].joinable()){threads[2].join();}
        
      }

      randomSleepTime(min_seconds_between_images, max_seconds_between_images);
      
      curl_global_cleanup();
    }
  }
  return 0;
}
