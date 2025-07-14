#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include "BYka.hpp"
#include "Node.hpp"
#include "Sockets.hpp"

constexpr int m = 6, N = 2, eta = 3, p = 31, q = 61;
constexpr int SERVER_PORT = 5000;
constexpr int PEER_PORT = 6000;
std::string SERVER_IP = "0.0.0.0";


void runServer(BYka& scheme) {
    std::string message;
    Sockets socket;
    int clientReq = 0;
    static int countId = 1; 
    std::vector<std::string> peerIPs;
    std::vector<int> peerPorts;
    if (!socket.bindAndListen(SERVER_PORT)) {
        std::cerr << "❌ Error al iniciar el servidor en el puerto " << SERVER_PORT << "\n";
        return;
    }

    while (true) {
    std::cout << "🟢 Servidor escuchando en el puerto " << SERVER_PORT << "...\n";
    std::string peer_ip;
    int peer_port;
    int clientFd = socket.acceptClient(&peer_ip, &peer_port);
    if (clientFd < 0) {
        std::cerr << "❌ Error al aceptar conexión del cliente.\n";
        return;
    }

    std::cout << "📥 Conexión entrante desde " << peer_ip << ":" << peer_port << "\n";

    if (socket.receiveStringAndInt(message, clientReq, clientFd)) {
        if (message == "request_id") {
            std::cout << "📩 Solicitud de ID recibida. Asignando ID: " << countId << "\n";
            if (!socket.sendStringAndInt("id", countId, clientFd)) {
                std::cerr << "❌ Error al enviar el ID al cliente.\n";
            } else {
                std::cout << "✅ ID enviado correctamente.\n";
                countId++;
                peerIPs.push_back(peer_ip);
                peerPorts.push_back(peer_port);
            }
        } else if (message == "request_ip") {
            int idBuscado=clientReq;

            std::cout << "📩 Solicitud de IP recibida para el ID: " << idBuscado << "\n";
            if (idBuscado < 1 || idBuscado >= countId) {
                std::cerr << "⚠️ ID no válido solicitado: " << idBuscado << "\n";
                if (!socket.sendStringAndInt("error", -1, clientFd)) {
                    std::cerr << "❌ Error al enviar el mensaje de error al cliente.\n";
                }
            } else {
                std::cout << "✅ Enviando IP del ID: " << idBuscado << "\n";
                if (!socket.sendStringAndInt(peerIPs[idBuscado-1], peerPorts[idBuscado-1], clientFd)) {
                    std::cerr << "❌ Error al enviar la IP al cliente.\n";
                } else {
                    std::cout << "✅ IP enviada correctamente: " << peerIPs[idBuscado-1] << ":" << peerPorts[idBuscado-1] << "\n";
                }

            }
        }
        else {
            std::cerr << "⚠️ Mensaje no reconocido: " << message << "\n";
        }
    } else {
        std::cerr << "❌ Error al recibir datos del cliente.\n";
    }
    }
}

// Servidor P2P de cada nodo
void runPeerServer(Node& me, BYka& scheme) {
    Sockets server;
    if (!server.bindAndListen(PEER_PORT)) {
        std::cerr << "❌ Nodo " << me.getID() << " no pudo abrir su puerto P2P.\n";
        return;
    }
    std::cout << "🛰️ Nodo " << me.getID() << " esperando peers en puerto " << PEER_PORT << "...\n";

    while (true) {
        std::string peer_ip;
        int peer_port;
        int peerFd = server.acceptClient(&peer_ip, &peer_port);
        if (peerFd < 0) continue;

        
        std::string msg;
        int val;
        if (server.receiveStringAndInt(msg, val, peerFd)) {
            std::cout << "🤝 Nodo " << me.getID() << " recibió: " << msg << " (" << val << ")\n";
            server.sendStringAndInt("ack", 200, peerFd);
        }

        server.closeSocket();
    }
}

void runClient(BYka& scheme) {
    // 1. Conectarse al servidor central y obtener ID
    Sockets socket;
    if (!socket.connectToServer(SERVER_IP, SERVER_PORT)) {
        std::cerr << "❌ No se pudo conectar al servidor central.\n";
        return;
    }

    socket.sendStringAndInt("request_id", 0);
    std::string response;
    int nodeId = -1;
    socket.receiveStringAndInt(response, nodeId);
    if (response != "id") {
        std::cerr << "❌ Respuesta inválida del servidor.\n";
        return;
    }

    Node me(nodeId, m, eta, N, p, q);
    scheme.assignKeysToNode(me);
    std::cout << "🔑 Nodo registrado con ID: " << nodeId << "\n";

    // 2. Iniciar servidor P2P en segundo hilo
    std::thread peerServerThread(runPeerServer, std::ref(me), std::ref(scheme));
    //std::this_thread::sleep_for(std::chrono::seconds(2));

    // 3. Solicitar conexión a otros peers
    while (true) {
        std::cout << "Ingrese ID de nodo a conectar (0 para salir): ";
        int targetId;
        std::cin >> targetId;
        if (targetId == 0) break;

        if (!socket.connectToServer(SERVER_IP, SERVER_PORT)) {
           std::cerr << "❌ No se pudo conectar al servidor central.\n";
              continue;
        }

        socket.sendStringAndInt("request_ip", targetId);
        std::cout << "🔍 Buscando IP del nodo " << targetId << "...\n";
        std::string peerIP;
        int peerPort=-1;
        if (!socket.receiveStringAndInt(peerIP, peerPort)) {
            std::cerr << "❌ Error al solicitar IP del nodo " << targetId << ".\n";
            continue;
        }
        std::cout << "Respuesta del servidor, dirección: " << peerIP << ":" << peerPort << "\n";
        if (peerPort <= 0 || peerIP.empty()) {
            std::cerr << "❌ No se pudo obtener IP del nodo.\n";
            continue;
        }

        std::cout << "➡️ Conectando con nodo " << targetId << " en " << peerIP << ":" << peerPort << "\n";
        Sockets peer;
        if (!peer.connectToServer(peerIP, peerPort)) {
            std::cerr << "❌ Falló conexión con nodo " << targetId << "\n";
            continue;
        }

        peer.sendStringAndInt("saludos_de", nodeId);
        std::string resp;
        int code;
        if (peer.receiveStringAndInt(resp, code)) {
            std::cout << "📨 Nodo " << targetId << " respondió: " << resp << " (" << code << ")\n";
        }

        peer.closeSocket();
    }

    peerServerThread.join();
}

int main(int argc, char* argv[]) {
    BYka scheme(m, N, eta, p, q);

    if (argc > 1 && std::string(argv[1]) == "server") {
        runServer(scheme);
    } else {
        SERVER_IP = argv[1];
        runClient(scheme);
    }

    return 0;
}
