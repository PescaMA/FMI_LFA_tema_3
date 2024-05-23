#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

/// context-free grammar
class CFG{
protected:
    static const char LAMBDA = '@';
    std::unordered_map<std::string,std::unordered_set<std::string> > productions;
    /// non-terminal -> {set of productions}.
    std::string startNonTerminal;

public:
    bool isEmpty()const{return productions.empty();}
    void clear(){
        productions.clear();
        startNonTerminal = "";
    }
    friend std::ostream& operator<<(std::ostream& out,const CFG& cfg){
        if(cfg.isEmpty())
            return out << "Grammar is empty!\n";

        out << "The start symbol is " << cfg.startNonTerminal;
        for(auto &productionSet:cfg.productions){
            out << "\n" << productionSet.first << " -> ";
            for(std::string production: productionSet.second){
                out << " | " << production;
            }
        }
        return out;
    }
};

class ChomskyException : public std::exception{
    int pos;
    std::string msg;
    public:
    ChomskyException(int pos,std::string msg):pos(pos),msg(msg){}
    std::string what(){
        return (msg + " at index " + std::to_string(pos));
    }
};

/// context-free grammar in Chomsky normal form
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
        int getI()const{return static_cast<int>(i);}
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


        if(!isalpha(info.getVal()) && info.getVal()!=LAMBDA)
            throw ChomskyException(info.getI(),"A production doesn't start with a letter!");

        if(info.getVal() == LAMBDA)
            info++;
        else
        if(islower(info.getVal())){
            info++;
            while(info.inString() && isdigit(info.getVal()))
                info++;
        }
        else
        if(isupper(info.getVal())){
            info++;
            while(info.inString() && isdigit(info.getVal()))
                info++;

            if(!info.inString() || !isupper(info.getVal()))
                throw ChomskyException(info.getI(),"A production has only one nonTerminal!");
            info++;

            while(info.inString() && isdigit(info.getVal()))
                info++;
        }
        updateProductions(info);

        if(info.inString() && info.getVal() != '|')
            throw ChomskyException(info.getI(),"Production not separated correctly!");

        info++;
        checkChomskyProduction(info);
    }


public:
    void addChomsky(std::string line){
        Info info(line);

        if(!isupper(info.getVal()))
            throw ChomskyException(info.getI(),"Production not starting with non-terminal!");
        info++;

        while(info.inString() && isdigit(info.getVal()))
            info++;

        info.setLeftProduction();
        if(startNonTerminal == "")
            startNonTerminal = info.getProduction();


        if(!info.inString() || info.getVal() != '-')
            throw ChomskyException(info.getI(),"No arrow (-) given!");
        info++;
        if(!info.inString() || info.getVal() != '>')
            throw ChomskyException(info.getI(),"No arrow (>) given!");
        info++;

        if(!info.inString() || (!isalpha(info.getVal()) && info.getVal() != LAMBDA ))
            throw ChomskyException(info.getI(),"No productions given!");

        checkChomskyProduction(info);

    }


    bool accept(std::string word){
        if(isEmpty())
            return false;
        std::vector< std::vector< std::unordered_set<std::string> > > reachable;

        int n = word.size();
        std::unordered_map<std::string,std::unordered_set<std::string> > reverseProductions;
        for(auto &productionSet : productions){
            for(std::string production : productionSet.second){
                 reverseProductions[production].insert(productionSet.first);
            }
        }
        ///for(int i=0;i<n;i++)

    }
    friend std::istream& operator>>(std::istream& in, ChomskyCFG& ccfg){
        /// FORMAT: line count n, then n lines. First line is the start.

        int n;
        in >> n;
        in.get();
        ccfg.clear();
        try{
            while(n--){
                std::string test;
                getline(in,test);
                ccfg.addChomsky(test);
            }
        }catch(ChomskyException& error){
            std::cerr << error.what() << '\n';
            ccfg.clear();
        }
        return in;
    }
};

int main(){


    ChomskyCFG cf;
    std::cin  >> cf;
    std::cout << cf;
    /*
    3
S -> AB | AA | c
B -> b
A -> @ | BA
*/
}
