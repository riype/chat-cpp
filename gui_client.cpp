#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

using namespace std;

int clientSocket = -1;
mutex chatMutex;
vector<string> chatHistory;
bool isConnected = false;

void receiveMessages(){
    while(true){
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        int bytes = recv(clientSocket, buf, sizeof(buf) -1, 0);

        if (bytes <= 0) {
            lock_guard<mutex> lock(chatMutex);
            chatHistory.push_back("[SYSTEM]: Connection lost.");
            isConnected = false;
            break;
        }
        buf[bytes] = '\0';

        string msg(buf);
        if (!msg.empty() && msg.back() == '\n') {
            msg.pop_back();
        }
        {
            lock_guard<mutex> lock(chatMutex);
            chatHistory.push_back(msg);
        }
    }
}

int main(){
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "C++ Chat", NULL, NULL);
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    char inputBuffer[256] = "";
    char nameBuffer[50] = "";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Set window to fill the screen
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Chat Window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (!isConnected) {
            ImGui::Text("Welcome to the chat!");
            ImGui::Spacing();
            ImGui::InputText("Your Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));
            
            if (ImGui::Button("Connect")) {
                clientSocket = socket(AF_INET, SOCK_STREAM, 0);
                if (clientSocket != -1) {
                    sockaddr_in serverAddr;
                    memset(&serverAddr, 0, sizeof(serverAddr));
                    serverAddr.sin_family = AF_INET;
                    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change IP if needed
                    serverAddr.sin_port = htons(8080);

                    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) != -1) {
                        string nameMsg = string(nameBuffer) + "\n";
                        send(clientSocket, nameMsg.c_str(), nameMsg.length(), 0);
                        
                        isConnected = true;
                        {
                            lock_guard<mutex> lock(chatMutex);
                            chatHistory.push_back("[SYSTEM]: Successfully connected!");
                        }
                        thread t(receiveMessages);
                        t.detach();
                    } else {
                        lock_guard<mutex> lock(chatMutex);
                        chatHistory.push_back("[SYSTEM]: Connection failed.");
                        close(clientSocket);
                    }
                }
            }
        } else {
            // History area
            ImGui::BeginChild("History", ImVec2(0, -40), true);
            {
                lock_guard<mutex> lock(chatMutex);
                for (const string& msg : chatHistory) {
                    ImGui::TextUnformatted(msg.c_str());
                }
                // Auto-scroll to bottom
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                    ImGui::SetScrollHereY(1.0f);
                }
            }
            ImGui::EndChild();

            // Input field and button
            ImGui::InputText("##msg", inputBuffer, IM_ARRAYSIZE(inputBuffer));
            ImGui::SameLine();
            
            if (ImGui::Button("Send") && strlen(inputBuffer) > 0) {
                string msg = string(inputBuffer) + "\n";
                send(clientSocket, msg.c_str(), msg.length(), 0);
                inputBuffer[0] = '\0';
            }
        }

        ImGui::End();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
