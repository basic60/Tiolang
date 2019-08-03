#include<string>
#include<vector>
using namespace std;
namespace str
{
    string strip(const string& s) {
        string tmp = s;
        while(tmp.size() && (tmp[0] == '\n' || tmp[0] == '\r' || tmp[0] == '\t' || tmp[0] == ' '))
            tmp.erase(tmp.begin());
        while(tmp.size() && (tmp.back() == '\n' || tmp.back() == '\r'|| tmp.back() == '\t' || tmp.back() == ' ')) 
            tmp.erase(tmp.end() - 1);
        return tmp;
    }

    vector<string> split(const string& str, char target) {
        vector<string> ret;
        string tmp;
        for(auto i:str) {
            if (i == target) {
                ret.push_back(tmp);
                tmp = "";
            } else {
                tmp += i;
            }
        }
        ret.push_back(tmp);
        return ret;
    }

    vector<string> split(const string& str, char target, int num) {
        vector<string> ret;
        string tmp;int lim = 0;
        for(auto i:str) {
            if (i == target && lim < num) {
                ret.push_back(tmp);
                tmp = "";
                lim++;
            } else {
                tmp += i;
            }
        }
        ret.push_back(tmp);
        return ret;
    }

} // strP