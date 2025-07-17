#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <fstream>
#include <ctime>
#include "BYka.hpp"
#include "Node.hpp"
#include "Sockets.hpp"


// set parameters
constexpr int m = 24, N = 8, eta = 8, p = 31, q = 65521;
constexpr int SERVER_PORT = 5000;
constexpr int PEER_PORT = 6000;
std::string SERVER_IP = "0.0.0.0";
unsigned t0, t1;


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
        } else if (clientReq == 1111) { 
            // Abrir archivo y escribir el reporte recibido
            std::ofstream reportFile("peer_report.txt", std::ios::app);
            if (reportFile.is_open()) {
                reportFile << message << std::endl;
                reportFile.close();
                std::cout << "📝 Reporte guardado en peer_report.txt\n";
                socket.sendStringAndInt("report_saved", 1, clientFd);
            } else {
                std::cerr << "❌ No se pudo abrir el archivo para guardar el reporte.\n";
                socket.sendStringAndInt("report_error", -1, clientFd);
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
void runPeerServer(Node& me, BYka& scheme, int id = 0) {
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
        int peerId;
        if (server.receiveStringAndInt(msg, peerId, peerFd)) {
            std::cout << "🤝 Nodo " << me.getID() << " recibió: " << msg << " (" << peerId << ")\n";
            server.sendStringAndInt("ack", id, peerFd);
        }
        Node peerNodeObj(peerId, m, eta, N, p, q);
        peerNodeObj.generatePublicKeys();
        
        std::vector<int> pairWiseKey = scheme.derivePairwiseKey(me, peerNodeObj);
        
        int sum = 0;
        for (int key : pairWiseKey) {
            sum += key;
        }
        std::cout << "🔑 Clave derivada con nodo " << peerId << ": " << sum << "\n";
        int peerResponseKey = 0;
        server.receiveStringAndInt(msg, peerResponseKey, peerFd);
        if (msg == "pairwise_key") {
            server.sendStringAndInt("pairwise_key", sum, peerFd);
        }
        std::cout << "📬 Respuesta del nodo " << peerId << ": " << msg << " (" << peerResponseKey << ")\n";
        bool success = sum == peerResponseKey;
        if (success) {
            std::cout << "✅ Clave derivada verificada correctamente con nodo " << peerId << "\n";
        } else {
            std::cout << "❌ Error de verificación de clave derivada con nodo " << peerId << "\n";
        }
        //server.closeSocket();
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
    std::thread peerServerThread(runPeerServer, std::ref(me), std::ref(scheme),nodeId);
    std::this_thread::sleep_for(std::chrono::seconds(2));


    // 3. Solicitar conexión a otros peers
    int targetId=1;
    while (true) {
        //std::cout << "Ingrese ID de nodo a conectar (0 para salir): ";
        //std::cin >> targetId;
        if (targetId == 0) break;
        if (targetId == nodeId) {
            // No puedes conectarte a ti mismo
            continue;
        }
        if (!socket.connectToServer(SERVER_IP, SERVER_PORT)) {
           std::cerr << "❌ No se pudo conectar al servidor central.\n";
              continue;
        }
        
        socket.sendStringAndInt("request_ip", targetId);
        std::cout << "🔍 Buscando IP del nodo " << targetId << "...\n";
        std::string peerIP;
        int peerPort=6000;
        int res;
        if (!socket.receiveStringAndInt(peerIP, res)) {
            std::cerr << "❌ Error al solicitar IP del nodo " << targetId << ".\n";
            continue;
        }
        std::cout << "Respuesta del servidor, dirección: " << peerIP << ":" << res << "\n";
        if (res <= 0 || peerIP.empty()) {
            std::cerr << "❌ No se pudo obtener IP del nodo.\n";
            continue;
        }

        // iniciar key exchange
        t0 = clock();
        
        std::cout << "➡️ Conectando con nodo " << targetId << " en " << peerIP << ":" << peerPort << "\n";
        Sockets peer;
        if (!peer.connectToServer(peerIP, peerPort)) {
            std::cerr << "❌ Falló conexión con nodo " << targetId << "\n";
            continue;
        }
        
        peer.sendStringAndInt("saludos_de", nodeId);
        std::string resp;
        int peerNode;
        if (peer.receiveStringAndInt(resp, peerNode)) {
            std::cout << "📨 Nodo " << targetId << " respondió: " << resp << " (" << peerNode << ")\n";
        }
        else {
            std::cerr << "❌ Error al recibir respuesta del nodo " << targetId << "\n";
        }
        Node peerNodeObj(peerNode, m, eta, N, p, q);
        peerNodeObj.generatePublicKeys();
        
        std::vector<int> pairWiseKey = scheme.derivePairwiseKey(me, peerNodeObj);
        
        int sum = 0;
        for (int key : pairWiseKey) {
            sum += key;
        }
        t1 = clock();
        // fin de derivación de clave

        std::cout << "🔑 Clave derivada con nodo " << targetId << ": " << sum << "\n";
        
        // Enviar clave derivada al peer
        if (!peer.sendStringAndInt("pairwise_key", sum)) {
            std::cerr << "❌ Error al enviar clave derivada al nodo " << targetId << "\n";
        } else {
            std::cout << "✅ Clave derivada enviada correctamente a nodo " << targetId << "\n";
        }
        std::string peerResponse;
        int peerResponseKey;
        if (!peer.receiveStringAndInt(peerResponse, peerResponseKey)) {
            std::cerr << "❌ Error al recibir respuesta del nodo " << targetId << "\n";
        } else {
            std::cout << "📬 Respuesta del nodo " << targetId << ": " << peerResponse << " (" << peerResponseKey << ")\n";
        }
        bool success = sum == peerResponseKey;
        if (success) {
            std::cout << "✅ Clave derivada verificada correctamente con nodo " << targetId << "\n";
        } else {
            std::cout << "❌ Error de verificación de clave derivada con nodo " << targetId << "\n";
        }
        
        
        // REPORTAR A SERVIDOR QUE SE HA CONECTADO A UN PEER
        if (!socket.connectToServer(SERVER_IP, SERVER_PORT)) {
            std::cerr << "❌ No se pudo conectar al servidor central.\n";
            return;
            }

        double time = (double)(t1 - t0) / CLOCKS_PER_SEC;
        std::string mensaje = std::to_string(nodeId)+"," + std::to_string(targetId) + ","+std::to_string(sum)+ "," + std::to_string(success) + "," + std::to_string(time);
        // Reportar a servidor que se ha conectado a un peer
        if (!socket.sendStringAndInt(mensaje, 1111)) {
            std::cerr << "❌ Error al informar al servidor sobre conexión a peer.\n";
        } else {
            std::cout << "✅ Conexión a peer " << targetId << " informada al servidor.\n";
        }
        
        peer.closeSocket();
        targetId++;
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
