#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
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
    void parseChomskyProduction(Info& info){
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
        parseChomskyProduction(info);
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

        parseChomskyProduction(info);

    }


    bool accept(std::string word){
        if(isEmpty())
            return false;
        int n = word.size();

        std::unordered_map<std::string,std::unordered_set<std::string> > reverseProductions;
        for(auto &productionSet : productions){
            for(std::string production : productionSet.second){
                 reverseProductions[production].insert(productionSet.first);
            }
        }
        std::vector< std::vector< std::unordered_set<std::string> > > reachable(n);

        reachable[0].resize(n);
        for(int i = 0;i<n;i++){
            if(reverseProductions.find(std::string(1,word[i])) != reverseProductions.end())
                reachable[0][i] = reverseProductions[std::string(1,word[i])];
        }


        /// learned from https://www.youtube.com/watch?v=VTH1k-xiswM&ab_channel=EducationAboutStuff
        for(int i=1;i<n;i++){
            reachable[i].resize(n-i);
            /// i+1 = the amount of letters being processed.
            for(int m = 0;m< n - i;m++){
                /// m + 1 = the current window of size i + 1.
                /// example: for i+1 = 2, m+1 = 2, and word = abcde, we will have ab, bc, cd, de.

                for(int j = 0;j<i;j++){
                    /// j+i = the part where the word is split into [0,j] and [j+1,i].
                    /// example: abcd can be obtained as a + bcd, ab + cd, abc + d.
                    for(std::string leftReachable : reachable[i-j-1][m]){
                        for(std::string rightReachable : reachable[j][m+i-j]){
                            if(reverseProductions.find(leftReachable+rightReachable) != reverseProductions.end())
                                for(std::string crossReachable : reverseProductions[leftReachable+rightReachable]){
                                    reachable[i][m].insert(crossReachable);
                            }
                        }
                    }
                }
            }
        }
        return reachable[n-1][0].find(startNonTerminal) != reachable[n-1][0].end();
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


class PDA{
    /// acceptance by empty stack.
    /// DOES NOT HAVE LAMBDA TRANSITIONS! problem qn -> qn with lambda,A/AA could be an infinite loop that adds A to the stack.


    int startState = -999;
    static const char stackStartSymbol = '$';
    static const char LAMBDA = '@';

    std::unordered_map<int, std::unordered_map<char,std::vector<std::tuple<int,char,std::string> > > > transitions;
    /// transition[from_state][symbol]= vector({to_state,stackRemove,stackAdd});

    public:
    bool readAccept(std::istream& in){
        std::string word;
        in >> word;
        return accept(word);
    }
    bool accept(std::string word){

       struct Configuration{
           int state;
           size_t i; /// i is position in word
           std::string stk;
       };
       std::queue<Configuration> possibilities;
       possibilities.push({startState,0,std::string(1,stackStartSymbol)});

       while(!possibilities.empty()){

            Configuration conf = possibilities.front();
            int state = conf.state;
            size_t i = conf.i;
            std::string stk = conf.stk;
            possibilities.pop();

            if(i == word.size()){
                if(stk.empty())
                    return true;
                continue;
            }

            for(auto transition : transitions[state][word[i]]){

                int to_state = std::get<0>(transition);
                char stackRemove = std::get<1>(transition);
                std::string stackAdd = std::get<2>(transition);
                if(stackRemove != stk.back())
                    continue;

                if(stackAdd == std::string(1,LAMBDA))
                    possibilities.push({to_state,i+1,stk.substr(0,stk.size()-1)});
                else
                    possibilities.push({to_state,i+1,stk.substr(0,stk.size()-1) + stackAdd});
            }
       }
        return false;
    }

    std::istream& read(std::istream& in){
        int n;
        in >> n;
        while(n--){
            int qs,qf;
            in >> qs >> qf;
            if(startState == -999)
                startState = qs;
            char letter, stackRemove;
            std::string stackAdd;
            in >> letter >> stackRemove >> stackAdd;

            transitions[qs][letter].push_back(std::tuple<int,char,std::string>(qf,stackRemove,stackAdd));

        }
        return in;
    }
    friend std::istream& operator>>(std::istream& in,PDA& pda){
        return pda.read(in);
    }
};

/// finite states transducer
class FST{
///nu exista lambda

int startState = -999;
std::unordered_map< int, std::unordered_map<char, std::vector<std::pair<int,std::string> > > > transitions;
std::unordered_set<int> finalStates;

public:
    std::vector<std::string> translate(std::string word){
        std::vector<std::string> result;
        if(word.empty()) /// special case
            return result;

        struct Configuration{
           int state;
           size_t i; /// i is position in word
           std::string word;
       };

        std::queue<Configuration> q;
        /// state, index, result;
        q.push({startState,0,""});

        while(!q.empty()){

            int state = q.front().state;
            unsigned int i = q.front().i;
            std::string traducedWord = q.front().word;

            q.pop();

            if(i == word.size()){/// reached the end of the word. Finish or backtrack
                if(finalStates.find(state) != finalStates.end())
                    result.push_back(traducedWord);
                continue;
            }

            for(auto &transition : transitions[state][word[i]]){
                int to_state = transition.first;
                std::string langB = transition.second;

                q.push({to_state,i+1,traducedWord + langB});
            }

        }
        return result;
    }


    std::istream& read(std::istream& in){
        int n;
        in >> n;
        while(n--){
            int qs,qf;
            in >> qs >> qf;
            if(startState == -999)
                startState = qs;
            char letter;
            std::string langB;
            in >> letter >> langB ;

            transitions[qs][letter].push_back(std::pair<int,std::string>(qf,langB));

        }
        in >> n; /// final states count
        while(n--){
            int fin;
            in >> fin;
            finalStates.insert(fin);
        }
        return in;
    }
    friend std::istream& operator>>(std::istream& in,FST& fst){
        return fst.read(in);
    }
};

template <class T>
void printVec(std::ostream& out, std::vector<T> v){
    if(v.empty())
        out << "Empty!";
    for(T el : v)
        out << el << ' ';
    out << '\n';
}
void test1(){
    std::ifstream fin("input1.in");
    std::ofstream fout("input1.out");
    ChomskyCFG cf;
    fin  >> cf;
    int q;
    fin >> q;
    ///std::cout << cf;
    while(q--){
        std::string s;
        fin >> s;
        fout << cf.accept(s) << '\n';
    }

}
void test2(){
    PDA p;
    std::ifstream fin("input2.in");
    std::ofstream fout("input2.out");
    fin >> p;
    int q;
    fin >> q;
    while(q--){
        fout << p.readAccept(fin) << '\n';
    }
}
void test3(){
    std::ifstream fin("input3.in");
    std::ofstream fout("input3.out");
    FST fst;
    fin >> fst;
    int q;
    fin >> q;
    while(q--){
        std::string s;
        fin >> s;
        printVec(fout,fst.translate(s));
    }

}


int main(){
    test1();
    test2();
    test3();
}
