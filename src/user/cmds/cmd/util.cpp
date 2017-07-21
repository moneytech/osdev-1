#include "cmd.hpp"
#include <algorithm>
#include <sstream>
#include <btos/envvars.hpp>

using namespace std;

const string default_prompt="[$p]";
const string prompt_var="PROMPT";
const string cwd_var="CWD";
const string default_cwd="INIT:/";

uint64_t tempcounter=0;

string get_env(const string &name){
	return GetEnv(name);
}

string get_env(const string &name, const string &def_value){
	string ret=get_env(name);
	if(ret==""){
		set_env(name, def_value);
		return def_value;
	}
	return ret;
}

void set_env(const string &name, const string &value){
	bt_setenv(name.c_str(), value.c_str(), 0);
}

string get_prompt(){
	return get_env(prompt_var, default_prompt);
}

string get_cwd(){
	return get_env(cwd_var, default_cwd);
}

void set_cwd(const string &value){
	set_env(cwd_var, value);
}

string to_lower(const string &str){
	string ret(str);
	transform(str.begin(), str.end(), ret.begin(), ::tolower);
	return ret;
}

void trim(string& str){
	string::size_type pos = str.find_last_not_of(' ');
	if(pos != string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		if(pos != string::npos) str.erase(0, pos);
	}
	else{
		str.erase(str.begin(), str.end());
	}
}

vector<string> split(const string &str, char delim) {
	vector<string> elems;
	stringstream ss(str);
	string item;
	while(getline(ss, item, delim)) {
		trim(item);
		if(item!=""){
			elems.push_back(item);
		}
	}
	return elems;
}

vector<string> get_paths(){
    string path=get_env("PATH");
    vector<string> ret=split(path, ',');
    if(find(ret.begin(), ret.end(), ".") == ret.end()) ret.push_back(".");
    return ret;
}

bool ends_with(const string &str, const string &end){
    if (str.length() >= str.length()) {
        return (0 == str.compare(str.length() - end.length(), end.length(), end));
    } else {
        return false;
    }
}

string tempfile(){
    stringstream ret;
    string temppath=get_env("TEMP");
    if(!temppath.length()) temppath=".";
    ret << temppath << "/" << "temp_cmd_" << bt_getpid() << "_" << tempcounter << ".tmp";
    tempcounter++;
    string path=parse_path(ret.str());
    if(path.length()){
        FILE *fh=fopen(path.c_str(), "w");
        fclose(fh);
    }
    return path;
}
