#include <iostream>
#include <vector>
#include <string>
#include "BYka.hpp"
#include "Node.hpp"
#include "Sockets.hpp"

constexpr int m = 6, N = 2, eta = 3, p = 31, q = 61;
constexpr int PORT = 5000;
const std::string SERVER_IP = "0.0.0.0";

void runServer(BYka& scheme) {
    std::string message;
    Sockets socket;
    int clientReq = 0;
    static int countId = 1;
    
    if (!socket.bindAndListen(PORT)) {
        std::cerr << "❌ Error al iniciar el servidor en el puerto " << PORT << "\n";
        return;
    }
    
    while (true){
    
    std::cout << "🟢 Servidor escuchando en el puerto " << PORT << "...\n";
    int clientFd = socket.acceptClient();
    if (clientFd < 0) {
        std::cerr << "❌ Error al aceptar conexión del cliente.\n";
        return;
    }

    std::cout << "🔗 Cliente conectado.\n";

    if (socket.receiveStringAndInt(message, clientReq, clientFd)) {
        if (message == "request_id") {
            std::cout << "📩 Solicitud de ID recibida. Asignando ID: " << countId << "\n";
            if (!socket.sendStringAndInt("id", countId, clientFd)) {
                std::cerr << "❌ Error al enviar el ID al cliente.\n";
            } else {
                std::cout << "✅ ID enviado correctamente.\n";
                countId++;
            }
        } else {
            std::cerr << "⚠️ Mensaje no reconocido: " << message << "\n";
        }
    } else {
        std::cerr << "❌ Error al recibir datos del cliente.\n";
    }
    }
}

void runClient(BYka& scheme) {
    Sockets socket;
    if (!socket.connectToServer(SERVER_IP, PORT)) {
        std::cerr << "❌ Error al conectar con el servidor en " << SERVER_IP << ":" << PORT << "\n";
        return;
    }

    std::cout << "🔌 Conectado al servidor. Solicitando ID...\n";
    if (!socket.sendStringAndInt("request_id", 0)) {
        std::cerr << "❌ Error al enviar solicitud de ID.\n";
        return;
    }

    std::string response;
    int nodeId = -1;
    if (!socket.receiveStringAndInt(response, nodeId)) {
        std::cerr << "❌ Error al recibir respuesta del servidor.\n";
        return;
    }

    if (response != "id") {
        std::cerr << "⚠️ Respuesta inesperada del servidor: " << response << "\n";
        return;
    }

    std::cout << "✅ ID recibido: " << nodeId << "\n";
    Node me(nodeId, m, eta, N, p, q);
    scheme.assignKeysToNode(me);

    std::cout << "Intentando conectarse con otros nodos " << me.getID() << ":\n";
    
}

int main(int argc, char* argv[]) {
    BYka scheme(m, N, eta, p, q);

    bool isServer = (argc > 1 && std::string(argv[1]) == "server");

    if (isServer) {
        runServer(scheme);
    } else {
        runClient(scheme);
    }

    return 0;
}
