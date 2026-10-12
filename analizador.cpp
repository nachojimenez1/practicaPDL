#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cctype>

using namespace std;

struct Simbolo {
    int id;
    string lexema;
    string tipo;
    int despl;
};

vector<Simbolo> tablaSimbolos;
map<string, int> mapaTS;
int desplazamientoActual = 0;

int insertarTS(const string& lexema, const string& tipo = "entero") {
    if (mapaTS.find(lexema) != mapaTS.end()) {
        return mapaTS[lexema];
    }
    int pos = tablaSimbolos.size() + 1;
    Simbolo s;
    s.id = pos;
    s.lexema = lexema;
    s.tipo = tipo;
    s.despl = desplazamientoActual;

    if (tipo == "real") desplazamientoActual += 4;
    else desplazamientoActual += 2;

    tablaSimbolos.push_back(s);
    mapaTS[lexema] = pos;
    return pos;
}

map<string, string> palabrasReservadas = {
    {"boolean", "BOOLEAN"},
    {"do", "DO"},
    {"echo", "ECHO"},
    {"float", "FLOAT"},
    {"function", "FUNCTION"},
    {"if", "IF"},
    {"int", "INT"},
    {"let", "LET"},
    {"prompt", "PROMPT"},
    {"return", "RETURN"},
    {"string", "STRING"},
    {"void", "VOID"},
    {"while", "WHILE"},
    {"false", "FALSE"},
    {"true", "TRUE"}
};

struct Token {
    string codigo;
    string atributo;
};

int main(int argc, char* argv[]) {
    string nombreEntrada = "entrada.txt";
    if (argc > 1) {
        nombreEntrada = argv[1];
    }

    ifstream entrada(nombreEntrada);
    if (!entrada.is_open()) {
        cerr << "Error: No se pudo abrir " << nombreEntrada << endl;
        return 1;
    }

    string fuente((istreambuf_iterator<char>(entrada)), istreambuf_iterator<char>());
    entrada.close();

    vector<Token> listaTokens;
    vector<string> erroresLexicos;
    size_t i = 0;
    size_t n = fuente.length();
    int linea = 1;

    bool esperandoTipo = false;
    string ultimoIdDeclarado = "";

    while (i < n) {
        char c = fuente[i];

        if (c == '\n') {
            linea++;
            i++;
            continue;
        }

        // Espacios y tabuladores
        if (isspace(c)) {
            i++;
            continue;
        }

        // Comentario de línea (//)
        if (c == '/' && i + 1 < n && fuente[i + 1] == '/') {
            i += 2;
            while (i < n && fuente[i] != '\n') {
                i++;
            }
            continue;
        }

        // Cadenas con comillas dobles
        if (c == '"') {
            i++;
            string lexema = "";
            while (i < n && fuente[i] != '"' && fuente[i] != '\n') {
                lexema += fuente[i];
                i++;
            }
            if (i < n && fuente[i] == '"') {
                i++;
                listaTokens.push_back({"CADENA", "\"" + lexema + "\""});
            } else {
                erroresLexicos.push_back("Linea " + to_string(linea) + ": Cadena sin cerrar antes del fin de linea.");
            }
            continue;
        }

        // Números enteros y reales
        if (isdigit(c)) {
            string numStr = "";
            bool esReal = false;
            while (i < n && (isdigit(fuente[i]) || fuente[i] == '.')) {
                if (fuente[i] == '.') esReal = true;
                numStr += fuente[i];
                i++;
            }
            if (esReal) {
                listaTokens.push_back({"CTEREAL", numStr});
            } else {
                listaTokens.push_back({"CTEENTERA", numStr});
            }
            continue;
        }

        // Identificadores y Palabras Reservadas
        if (isalpha(c)) {
            string lexema = "";
            while (i < n && isalnum(fuente[i])) {
                lexema += fuente[i];
                i++;
            }

            if (palabrasReservadas.find(lexema) != palabrasReservadas.end()) {
                string cod = palabrasReservadas[lexema];
                listaTokens.push_back({cod, "-"});

                if (esperandoTipo) {
                    string tipoTS = "entero";
                    if (lexema == "int") tipoTS = "entero";
                    else if (lexema == "float") tipoTS = "real";
                    else if (lexema == "string") tipoTS = "cadena";
                    else if (lexema == "boolean") tipoTS = "logico";

                    for (auto& s : tablaSimbolos) {
                        if (s.lexema == ultimoIdDeclarado) {
                            s.tipo = tipoTS;
                            break;
                        }
                    }
                    esperandoTipo = false;
                }
            } else {
                int pos = insertarTS(lexema);
                listaTokens.push_back({"ID", to_string(pos)});
                ultimoIdDeclarado = lexema;
                esperandoTipo = true;
            }
            continue;
        }

        // Operadores y Delimitadores (Nombres exactos de Draco)
        if (c == '-' && i + 1 < n && fuente[i + 1] == '=') {
            listaTokens.push_back({"ASIGRESTA", "-"});
            i += 2;
        } else if (c == '=') {
            listaTokens.push_back({"ASIG", "-"});
            i++;
        } else if (c == '+') {
            listaTokens.push_back({"SUMA", "-"});
            i++;
        } else if (c == '&' && i + 1 < n && fuente[i + 1] == '&') {
            listaTokens.push_back({"AND", "-"});
            i += 2;
        } else if (c == '<') {
            listaTokens.push_back({"MENOR", "-"});
            i++;
        } else if (c == ';') {
            listaTokens.push_back({"PUNTOCOMA", "-"});
            esperandoTipo = false;
            i++;
        } else if (c == ',') {
            listaTokens.push_back({"COMA", "-"});
            i++;
        } else if (c == '(') {
            listaTokens.push_back({"PARABRE", "-"});
            i++;
        } else if (c == ')') {
            listaTokens.push_back({"PARCIERRA", "-"});
            i++;
        } else if (c == '{') {
            listaTokens.push_back({"LLAVEABRE", "-"});
            i++;
        } else if (c == '}') {
            listaTokens.push_back({"LLAVECIERRA", "-"});
            i++;
        } else {
            erroresLexicos.push_back("Linea " + to_string(linea) + ": Caracter no permitido: '" + string(1, c) + "'");
            i++;
        }
    }

    listaTokens.push_back({"EOF", "-"});

    if (!erroresLexicos.empty()) {
        cout << "SE HAN DETECTADO ERRORES LEXICOS:\n";
        for (const auto& err : erroresLexicos) {
            cout << "  -> " << err << "\n";
        }
        return 0;
    }

    // Volcado de tokens.txt
    ofstream fTokens("tokens.txt");
    for (const auto& t : listaTokens) {
        fTokens << "<" << t.codigo << ", " << t.atributo << ">\n";
    }
    fTokens.close();

    // Volcado de ts.txt
    ofstream fTS("ts.txt");
    fTS << "#1:\n";
    for (const auto& s : tablaSimbolos) {
        fTS << "* '" << s.lexema << "'\n";
        fTS << "+ tipo: '" << s.tipo << "'\n";
        fTS << "+ despl: " << s.despl << "\n";
    }
    fTS.close();

    cout << "Generados 'tokens.txt' y 'ts.txt' sincronizados con Draco.\n";
    return 0;
}