#include <iostream>
#include "uPascal.h"
#include <string>
#include <map>
#include <sstream> 
#include <cstdio> 
#include <bitset> 

extern FILE* yyin;

Uzel* Evaluate(Uzel*);
int Interpr(const Uzel*);

Uzel* Koren;
char Source[81];

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }

    strncpy(Source, argv[1], sizeof(Source) - 1);

    if (!(yyin = fopen(argv[1], "rt"))) {
        std::cerr << "Error: Cannot open source file '" << argv[1] << "'" << std::endl;
        return 1;
    }

    Flush();
    LexInic();

    int parse = yyparse();

    fclose(yyin);
    Interpr(Koren);

    return 0;
}


void Chyba(const char* message, Pozice position) {
    std::cerr << "Error at: " << position << ", Message: " << message << std::endl;
    exit(1);
}

Uzel* GenUzel(int Typ, Uzel* prvni, Uzel* druhy, Uzel* treti, Uzel* ctvrty) {
    Uzel* uzel = new Uzel;
    uzel->Typ = Typ;
    uzel->z.z.prvni = prvni;
    uzel->z.z.druhy = druhy;
    uzel->z.z.treti = treti;
    uzel->z.z.ctvrty = ctvrty;
    return uzel;
}

Uzel* GenCislo(int value) {
    Uzel* uzel = new Uzel;
    uzel->Typ = CISLO; 
    uzel->z.Cislo = value;
    return uzel;
}

Uzel* GenRetez(const char* str) {
    Uzel* uzel = new Uzel;
    uzel->Typ = RETEZ; 
    uzel->z.Retez = new char[strlen(str)+1];
    memcpy((void*)uzel->z.Retez, str, strlen(str) + 1);
    return uzel;
}

std::map<std::string, int*> variables;
Uzel* GenPromen(const char* name) {
    std::string str = name;
    Uzel* uzel = new Uzel;
    uzel->Typ = PROMENNA;

    auto it = variables.find(str);
    if (it == variables.end()) {
        uzel->z.Adresa = new int;
        variables[str] = uzel->z.Adresa;
    }
    else {
        uzel->z.Adresa = it->second;
    }

    return uzel;
}

void yyerror(const char* message) {
    std::cerr << "Parser Error: " << message << std::endl;
    exit(1);
}

void handleWrite(const Uzel* u) {
    Uzel* node = const_cast<Uzel*>(u->z.z.prvni);
    std::string str;

    if (node->Typ == RETEZ) {
        str = node->z.Retez;
    }
    else if (node->Typ == CHR) {
        int value = Interpr(node->z.z.prvni);
        str = std::string(1, static_cast<char>(value));
        
    }
    else if (node->Typ == HEX) {
        int value = Interpr(node->z.z.prvni);
        std::stringstream ss;
        ss << std::hex << value;
        str = ss.str();
        
    }
    else if (node->Typ == BIN) {
        int value = Interpr(node->z.z.prvni);
        std::string binaryString = std::bitset<32>(value).to_string();
        
        size_t pos = binaryString.find_first_not_of('0');
        if (pos != std::string::npos) {
            str = binaryString.substr(pos);
        }
        else {
            str = "0"; 
        }
    }
    else {
        int value = Interpr(node);
        str = std::to_string(value);
    }

    if (u->z.z.druhy != nullptr) {
        int padding = Interpr(u->z.z.druhy) - str.length();
        if (padding > 0) {
            std::cout << std::string(padding, ' ');
        }
    }
    std::cout << str << std::flush;
}


int Interpr(const Uzel * u) {
        if (u == NULL) return 0;
#define prvni u->z.z.prvni
#define druhy u->z.z.druhy
#define treti u->z.z.treti
#define ctvrty u->z.z.ctvrty

        switch (u->Typ) {
        case 0: Interpr(prvni); Interpr(druhy);
            return 0;
        case PRIRAZENI:
            *prvni->z.Adresa = Interpr(druhy);
            return 0;
        case RETEZ:
            return 0;
        case IF:
            if (Interpr(prvni)) Interpr(druhy);
            else Interpr(treti);
            return 0;
        case NOT:
            return ~Interpr(prvni);
        case OR:
            return Interpr(prvni) | Interpr(druhy);
        case '<':
            return Interpr(prvni) < Interpr(druhy);
        case '>':
            return Interpr(prvni) > Interpr(druhy);
        case '-':
            if (druhy) return Interpr(prvni) - Interpr(druhy);
            return -Interpr(prvni);
        case '/': {
            int d = Interpr(druhy);
            if (d == 0) { printf("\nDeleni nulou\n"); abort(); }
            return Interpr(prvni) / d;
        }
        
        case '*': {
            int d = Interpr(druhy);
            return Interpr(prvni) * d;
        }
        
        case '+': {
            int d = Interpr(druhy);
            return Interpr(prvni) + d;
        }
        
        case '=': {
            int d = Interpr(druhy);
            return Interpr(prvni) == d;
        }
        
        case VETSIROVNO:
            return Interpr(prvni) >= Interpr(druhy);
        case MENSIROVNO:
            return Interpr(prvni) <= Interpr(druhy);
        case NENIROVNO:
            return Interpr(prvni) != Interpr(druhy);
        case XOR:
            return (Interpr(prvni) && !Interpr(druhy)) || (!Interpr(prvni) && Interpr(druhy));
        case '!':
            return Interpr(prvni) ^ Interpr(druhy);
        case '|':
            return Interpr(prvni) | Interpr(druhy);
        case SHR:
            return Interpr(prvni) >> Interpr(druhy);
        case SHL:
            return Interpr(prvni) << Interpr(druhy);
        case AND:
            return Interpr(prvni) & Interpr(druhy);
        case '&':
            return Interpr(prvni) & Interpr(druhy);

        case CISLO:
            return u->z.Cislo;

        case PROMENNA:
            return *u->z.Adresa;
        case WRITELN:
            if (prvni == 0) {
                std::cout << std::endl;
                return 0;
            }
            handleWrite(u);
            std::cout << std::endl;
            return 0;
        case WRITE:
            handleWrite(u);
            return 0;
        case REPEAT: 
            do {
                Interpr(prvni);
            } while (!Interpr(druhy));
            return 0;
        case WHILE: 
            while (Interpr(prvni)) {
                Interpr(druhy);  
            }
            return 0;
        case MOD: {
            int divisor = Interpr(druhy);
            if (divisor == 0) {
                throw std::invalid_argument("Division by zero");
            }
            else {
                int base = Interpr(prvni);
                return Interpr(prvni) % Interpr(druhy);
            }
            return 0;
        }
        case FOR: {
            
            int start = Interpr(druhy);
            int end = Interpr(treti);
            *prvni->z.Adresa = start;
            for (*prvni->z.Adresa; *prvni->z.Adresa <= end; (*prvni->z.Adresa)++) {
                Interpr(ctvrty);
            }
            return 0;
        }

        case FORDOWN: {
            int start = Interpr(druhy);
            int end = Interpr(treti);
            for (*prvni->z.Adresa = start; *prvni->z.Adresa > end; (*prvni->z.Adresa)--) {
                Interpr(ctvrty);
            }
            return 0;
        }
        case ORD: {
            const Uzel* evalNode = prvni;  

            if (evalNode->Typ == RETEZ) {
                const char* retez = evalNode->z.Retez;
                if (strlen(retez) > 0) {
                    return static_cast<int>(retez[0]);  
                }
                else {
                    throw std::invalid_argument("ORD called with an empty string");
                }
            }
            return 0;
        }
        case READ:
        {
            int* promenna = prvni->z.Adresa;
            std::cin >> *promenna;
            break;
        }
        
        return 0;

        }
}

