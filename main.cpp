#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

/// context-free grammar
class CFG{
protected:
    std::map<std::string,std::set<std::string> > productions;
    std::string startNonTerminal;


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

    struct Info{
    protected:
        size_t start=0;
        size_t i = 0;
        std::string leftProduction;
        std::string productions;

    public:
        Info(std::string line):productions(line){
            productions.erase(std::remove(productions.begin(),productions.end(),' '),productions.end());
        }
        const std::string getProduction() const {
            return productions.substr(start,i - start);
        }
        void setLeftProduction(){
            leftProduction = getProduction();
        }
        const std::string& getLeftProduction() const {return leftProduction;}
        void setStartOfProduction(){start = i;}

        bool inString() const{return i < productions.size();}
        void operator++(int){i++;}
        char getVal() const{return productions[i];}
    };
    void updateProductions(const Info& info){
        productions[info.getLeftProduction()].insert(info.getProduction());
    }
    void checkChomskyProduction(Info& info){
        if(!info.inString())
            return;

        info.setStartOfProduction();


        if(!isalpha(info.getVal()))
            throw ChomskyException("A production doesn't start with a letter!");

        if(islower(info.getVal())){
            info++;
            while(info.inString() && isdigit(info.getVal()))
                info++;
        }
        if(isupper(info.getVal())){
            info++;
            while(info.inString() && isdigit(info.getVal()))
                info++;

            if(!info.inString() || !isupper(info.getVal()))
                throw ChomskyException("A production has only one nonTerminal!");
            info++;

            while(info.inString() && isdigit(info.getVal()))
                info++;


        }
        updateProductions(info);

        if(info.inString() && info.getVal() != '|')
            throw ChomskyException("Production not separated correctly!");

        info++;
        checkChomskyProduction(info);
    }


public:
    void checkChomsky(std::string line){

        try{
        Info info(line);

        if(!isupper(info.getVal()))
            throw ChomskyException("Production not starting with non-terminal!");
        info++;

        while(info.inString() && isdigit(info.getVal()))
            info++;

        info.setLeftProduction();

        if(!info.inString() || info.getVal() != '-')
            throw ChomskyException("No arrow (-) given!");
        info++;
        if(!info.inString() || info.getVal() != '>')
            throw ChomskyException("No arrow (>) given!");
        info++;

        if(!info.inString() || !isalpha(info.getVal()))
            throw ChomskyException("No productions given!");

        checkChomskyProduction(info);
        }catch(ChomskyException& error){
            std::cerr << error.what() << '\n';
        }

    }
    bool accept(std::string word){
        return false;
    }
};

int main(){
    std::string test;
    getline(std::cin,test);
    ChomskyCFG cf;
    cf.checkChomsky(test);
}
