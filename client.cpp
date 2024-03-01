#include <iostream>
#include <winsock2.h>
using namespace std;

class Client{
public:
    WSADATA WSAData;
    SOCKET server;
    SOCKADDR_IN addr;
    char buffer[1024];
    boolean activo = true;
    int puerto;
    boolean solicitoLog = false;
    Client()
    {
        cout<<"Ingrese el puerto al que quiere conectarse: ";
        cin>>puerto;
        cout<<"Conectando al servidor..."<<endl<<endl;
        WSAStartup(MAKEWORD(2,0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);
        addr.sin_addr.s_addr = inet_addr("192.168.1.52");
        addr.sin_family = AF_INET;
        addr.sin_port = htons(puerto);
        while(connect(server, (SOCKADDR *)&addr, sizeof(addr)) == -1){
            cout<<"Ha ingresado mal el puerto"<<endl;
            cout<<"Ingrese un puerto nuevamente: ";
            cin>>puerto;
            addr.sin_port = htons(puerto);
        }
        cout << "Conectado al Servidor!" << endl<<endl<<endl;
        
    }
    void Enviar()
    {
        string mensaje = "";
        string opcion = "";
        boolean salir = false;
        do{
            cout<<"Por favor, ingrese una de las siguientes opciones\n";
            cout<<"Opcion 1: Realizar calculo\n";
            cout<<"Opcion 2: Ver registro de actividades\n";
            cout<<"Opcion 3: Cerrar sesion\n";
            cin>>opcion;
            if (opcion.length()>1){
                opcion = "4";
            }
            switch (opcion.front()){
                case '1':
                    system("cls");
                    cout<<"Ingrese el calculo: (o escriba 'volver' si desea regresar)\n";
                    cin.ignore();
                    getline(cin,mensaje);
                    if (mensaje=="volver"){
                        break;
                    }
                    mensaje.insert(0,1,'a');
                    strcpy(buffer, mensaje.c_str());
                    salir = true;
                    break;
                case '2':
                    mensaje = 'b';
                    strcpy(buffer, mensaje.c_str());
                    solicitoLog = true;
                    salir = true;
                    break;
                case '3':
                    mensaje = 'c';
                    strcpy(buffer, mensaje.c_str());
                    send(server, buffer, sizeof(buffer), 0);
                    memset(buffer, 0, sizeof(buffer));
                    CerrarSocket();
                    this->activo = false;
                    salir = true;
                    break;
                default:
                    system("cls");
                    cout<<"Error, ha ingresado un caracter incorrecto\n\n";
                    break;
            }
        } while (!salir);
        system("cls");
        send(server, buffer, sizeof(buffer), 0);
        memset(buffer, 0, sizeof(buffer));
        cout << "El mensaje ha sido enviado." << endl;
    }
    int Recibir()
    {
        int rec = recv(server, buffer, sizeof(buffer), 0);
        if (rec==-1){
            cout << "Cliente desconectado por inactividad " << endl;
            system("pause");
        } else {  
            if (solicitoLog){
                cout << "El servidor dice: \n" << "\n\n";
                string bufferString = buffer;
                int archivoLogTamanio = (int)(bufferString[0]);
                for (int i=0; i<archivoLogTamanio-1; i++){
                    bufferString = buffer;
                    cout << bufferString.erase(0,1);
                    recv(server, buffer, sizeof(buffer), 0);
                }
                bufferString = buffer;
                cout << bufferString.erase(0,1)<<endl;
                solicitoLog = false;
            } else {
                cout << "El servidor dice: \n" << buffer << "\n\n";
            }
        }
        memset(buffer, 0, sizeof(buffer));
        return rec;
    }
    void CerrarSocket()
    {
       closesocket(server);
       WSACleanup();
    }
};

int main()
{
    Client *Cliente = new Client();
    while(Cliente->activo)
    { 
        Cliente->Enviar();
        if (Cliente->activo){
            if (Cliente->Recibir() == -1){
                Cliente = new Client();
            }
        }
    }
    system("cls");
    cout << "Se ha cerrado la conexion." << endl;
    system("pause");
}