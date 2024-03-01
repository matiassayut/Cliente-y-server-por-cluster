#include <iostream>
#include <winsock2.h>
#include <chrono>
#include <ctime>
#include <string>
#include <tuple>
#include <math.h>
#include <cstring>
#include <fstream>
#define PORT 5000
using namespace std;

string fechaActual();
tuple <bool,string> validarCuenta(string cuenta);
string calcularCuenta(string cuenta);
void guardarStringEnArchivo(string texto);
string cargarArchivo();

class Server{
public:
    WSADATA WSAData;
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;
    char buffer[1024];
    string bufferString;
    Server()
    {
        WSAStartup(MAKEWORD(2,0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        DWORD timeout = 120 * 1000;
        setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
        bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
        listen(server, 0);
        string fecha = fechaActual();
        guardarStringEnArchivo(fecha + " =============================\n" +
        fecha + " =======Inicia Servidor=======\n" +
        fecha + " =============================\n" +
        fecha + " Socket creado. Puerto de escucha:" + to_string(PORT) + "\n");
        cout << fecha << " =============================" << endl;
        cout << fecha << " =======Inicia Servidor=======" << endl;
        cout << fecha << " =============================" << endl;
        cout << fecha << " Socket creado. Puerto de escucha:"<< PORT << endl;
        int clientAddrSize = sizeof(clientAddr);
        if((client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize)) != INVALID_SOCKET){   
            guardarStringEnArchivo(fechaActual() + " Conexion Aceptada.\n");
            cout << fechaActual() << " Conexion Aceptada." << endl;
        }
    }

    int Recibir()
    {
        int mensaje = recv(client, buffer, sizeof(buffer), 0);
        bufferString = buffer;
        memset(buffer, 0, sizeof(buffer));
        if (mensaje==-1 && bufferString[0]!='c'){
            guardarStringEnArchivo(fechaActual() + " Conexion Cerrada por Inactividad.\n");
            cout << fechaActual() << " Conexion Cerrada por Inactividad." << endl;
            CerrarSocket();
            EsperarCliente();
        }
        return mensaje;
    }
    void Enviar()
    {
        bool validacion;
        char opcion = bufferString[0];
        string calculo = "";
        string archivoLog;
        string archivoLogTemp;
        int archivoLogTamanio;
        switch (opcion){
            case 'a':
                calculo = bufferString.erase(0,1);
                guardarStringEnArchivo(fechaActual() + " El cliente solicito el calculo de: " + calculo + "\n");
                cout << fechaActual() << " El cliente solicito el calculo de: " << calculo << endl;
                validacion = get<0>(validarCuenta(calculo));
                if (validacion){
                    strcpy(buffer, ("Resultado: " + calcularCuenta(calculo)).c_str());
                    guardarStringEnArchivo(fechaActual() + " El resultado enviado es: " + calcularCuenta(calculo) + "\n");
                    cout << fechaActual() << " El resultado enviado es: " << calcularCuenta(calculo) << endl;
                } else {
                    strcpy(buffer, get<1>(validarCuenta(calculo)).c_str());
                    guardarStringEnArchivo(fechaActual() + " " + get<1>(validarCuenta(calculo)) + "\n");
                    cout << fechaActual() << " " << get<1>(validarCuenta(calculo)) << endl;
                }
                send(client, buffer, sizeof(buffer), 0);
                memset(buffer, 0, sizeof(buffer));
                break;
            case 'b':
                guardarStringEnArchivo(fechaActual() + " El cliente ha solicitado ver el archivo server.log\n");
                cout << fechaActual() << " El cliente ha solicitado ver el archivo server.log" << endl;
                archivoLog = cargarArchivo();
                archivoLogTamanio = (archivoLog.length()/1022) + 1;
                for (int i=0; i<archivoLogTamanio; i++){
                    archivoLogTemp = archivoLog.substr(i*1022,1022);
                    archivoLogTemp.insert(0,1,archivoLogTamanio);
                    strcpy(buffer, archivoLogTemp.c_str());
                    send(client, buffer, sizeof(buffer), 0);
                }
                memset(buffer, 0, sizeof(buffer));
                break;
            case 'c':
                guardarStringEnArchivo(fechaActual() + " Conexion Cerrada por el cliente\n");
                cout << fechaActual() << " Conexion Cerrada por el cliente" << endl;
                strcpy(buffer, "La conexion se ha cerrado.");
                send(client, buffer, sizeof(buffer), 0);
                memset(buffer, 0, sizeof(buffer));
                CerrarSocket();
                EsperarCliente();
                break;
            default:
                break;
        }
    }
    void CerrarSocket()
    {
        closesocket(client);
    }
    void EsperarCliente()
    {
        int clientAddrSize = sizeof(clientAddr);
        if((client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize)) != INVALID_SOCKET){
            guardarStringEnArchivo(fechaActual() + " Conexion Aceptada.\n");
            cout << fechaActual() << " Conexion Aceptada." << endl;
        }
    }
};


int main()
{
    Server *Servidor = new Server();
    while(true)
    {   
        Servidor->Recibir();
        Servidor->Enviar();
    }
}

string fechaActual(){
    string fecha;
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    fecha = to_string(now->tm_year + 1900) + "-" + to_string(now->tm_mon + 1) + "-" + to_string(now->tm_mday) + "_" + to_string(now->tm_hour) + ":" + to_string(now->tm_min);
    return fecha;
}

tuple <bool,string> validarCuenta(string cuenta){
    bool resultado  = true;
    string error;
    int contador = 0;
    int pos = 0;
    char operando;
    for (int i = 0; i<cuenta.length(); i++){
        if (!isdigit(cuenta[i])){
            pos = i;
            contador ++;
        }
    }
    operando = cuenta[pos];
    if (contador!=1 || (operando != '!' && pos==cuenta.length()-1) || (operando == '!' && pos!=cuenta.length()-1) || pos==0){
        resultado = false;
        error = "No se pudo realizar la operacion, la operacion esta mal formada: [";
        if (pos==cuenta.length()-1){
            error.append(1,cuenta[pos-1]);
            error.append(1,operando);
        } else  if (pos==0){
            error.append(1,operando);
            error.append(1,cuenta[pos+1]);
            } else {
            error.append(1,cuenta[pos-1]);
            error.append(1,operando);
            error.append(1,cuenta[pos+1]);
        }
        error.append(1,']');
    }
    if (operando !=  '+' && operando !=  '-' && operando != '*' && operando != '/' && operando != '!' && operando != '^'){
        resultado = false;
        error = "No se pudo realizar la operacion, se encontro un caracter no contemplado: ";
        error.append(1,operando);
    }
    if (contador==0){
        resultado = true;
        error = "";
    }
    if (cuenta.length()<1 || cuenta.length()>20){
        resultado = false;
        error = "La operacion debe tener entre 1 y 20 caracteres";
    }
    
    return make_tuple(resultado,error);
}

string calcularCuenta(string cuenta){
    int pos = 0;
    char operando;
    string resultado;
    int contador = 0;
    for (int i = 0; i<cuenta.length(); i++){
        if (!isdigit(cuenta[i])){
            pos = i;
            contador ++;
        }
    }
    if (contador == 0){
        resultado = cuenta;
    } else {
        operando = cuenta[pos];
        long long enteroAntes = stoll(cuenta.substr(0,pos));
        long long enteroDespues = 0;
        int fact=1;
        if (operando != '!'){
            enteroDespues = stoll(cuenta.substr(pos+1,cuenta.length()));
        }
        switch (operando){
            case '+': 
                resultado = to_string(enteroAntes + enteroDespues);
                break;
            case '-': 
                resultado = to_string(enteroAntes - enteroDespues);
                break;
            case '*': 
                resultado = to_string(enteroAntes * enteroDespues);
                break;
            case '/': 
                resultado = to_string(enteroAntes / enteroDespues);
                break;
            case '!':
                for(int i = 1;i<=enteroAntes;i++){
                    fact = fact * i;
                }
                resultado = to_string(fact);
                break;
            case '^':
                resultado = to_string((int)(pow(enteroAntes,enteroDespues)));
                break;
            default:
                break;
        }
    }
    return resultado;
}

void guardarStringEnArchivo(string texto){
    ofstream file("server.log.txt",ios::app);
    file << texto;
    file.close();
}

string cargarArchivo(){
    string texto;
    string textFromFile;
    string resultado;
    ifstream file("server.log.txt");
    while (getline (file, textFromFile)) {
        texto.append(textFromFile + "\n");
    }
    file.close();
    return texto;
}