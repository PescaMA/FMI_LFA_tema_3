#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

/// context-free grammar
class CFG{

};
/*
void trim(std::string& s){
    size_t i;
    for(i = 0; i < s.size() ;i++){
        if(s[i] != ' ')
            break;
    s.erase(s.begin(),i);

    for(i = s.size()-1; i > 0 ;i--){
        if(s[i] != ' ')
            break;
    }
    s.erase(s.begin() + i,s.end());
}*/
/// context-free grammar in Chomsky normal form

class ChomskyException : public std::exception{
    std::string msg;
    public:
    ChomskyException(std::string msg):msg(msg){}
    const char* what(){return msg.c_str();}
};

class ChomskyCFG : public CFG{

    void checkChomskyProduction(size_t &i,std::string& productions){
        if(i >= productions.size())
            return;

        if(isalpha(productions[i]))
            throw ChomskyException("A production doesn't start with a letter!");
        i++;

        if(!isupper(productions[i])){
            while(i < productions.size() && isdigit(productions[i]))
                i++;
        }
        else{
            while(i < productions.size() && isdigit(productions[i]))
                i++;

            if(i == productions.size() || !isupper(productions[i]))
                throw ChomskyException("A production has only one nonTerminal!");

            while(i < productions.size() && isdigit(productions[i]))
                i++;

        }
        if(productions[i] != '|')
            throw ChomskyException("Production not separated correctly!");

        i++;
        checkChomskyProduction(i,productions);
    }
    void checkChomsky(std::string line){
        line.erase(std::remove(line.begin(),line.end(),' '),line.end());
        std::string left;

        if(!isupper(line[0]))
            throw ChomskyException("Production not starting with non-terminal!");

        size_t i = 1;
        while(i < line.size() && isdigit(line[i]))
            i++;

        if(i == line.size() || !isalpha(line[i]))
            throw ChomskyException("No productions given!");

        return checkChomskyProduction(i,line);
    }
    bool accept(std::string word){

    }
};

int main(){

}
