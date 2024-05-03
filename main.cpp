#include <conio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <chrono>
#include <thread>
#include <Windows.h>

#include "socklib.h"
#include "constants.h"
#include "udp_client.h"
#include "tests.h"

// Use this function in main() to try simple tests.
int udp_assignment() {
  {
    std::cout << "Testing without request ids...\n";
    UDPClient client(consts::HOST, consts::ECHO_PORT);
    std::string result;
    int error = client.send_message_by_character("Beautiful is better than ugly", result);
    if (error == -1) {
      std::cout << "Timed out.\n";
      return -1;
    } else {
      std::cout << "Got result '" << result << "'.\n";
    }
  }

  {
    std::cout << "Testing without request ids...\n";
    UDPClient client(consts::HOST, consts::REQUEST_ID_PORT, true);
    std::string result;
    int error = client.send_message_by_character("To be or not to be, that is the question", result);
    if (error == -1) {
      std::cout << "Timed out.\n";
      return -1;
    } else {
      std::cout << "Got result '" << result << "'.\n";
    }
  }

  return 0;
}

template<typename T>
size_t copy_to_buffer(char* buffer, T* value, size_t buffer_len) {
    if (sizeof(T) > buffer_len) return 0;
    char* as_bytes = (char*)value;
    for (int i = 0; i < sizeof(T); i++) {
        buffer[i] = as_bytes[i];
    }
    return sizeof(T);
}

template<typename T>
size_t read_from_buffer(char* buffer, T* out_value) {
    memcpy(out_value, buffer, sizeof(T));
    return sizeof(T);
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

std::string is_valid_ip_address(const std::string& ip_address)
{
    std::vector<std::string> ip_address_parts = split(ip_address, '.');
    std::string good = ip_address;
    int asInt = -1;
    if (ip_address_parts.size() == 4)
    {
        for (int j = 0; j < ip_address_parts.size(); j++)
        {
            if (ip_address_parts[j].find_first_not_of("0123456789") == std::string::npos)
            {
                asInt = std::stoi(ip_address_parts[j]);
                if (asInt < 0 || asInt > 255)
                    good = "fail";
            }
            else {
                good = "fail";
            }
        }
    }
    else
    {
        good = "fail";
    }
    return good;
}

const char WALL = 178;
const char PATH = 176;
const char DOOR = 177;
const char PLAYER = 143;

enum DOORS {
    bN = 1,
    bE = 2,
    bS = 4,
    bW = 8,
    eN = 16,
    eE = 32,
    eS = 64,
    eW = 128,
};

struct Room
{
    char doors;

    bool getNorth()
    {
        return (doors & bN) == bN;
    }
    bool getEast()
    {
        return (doors & bE) == bE;
    }
    bool getSouth()
    {
        return (doors & bS) == bS;
    }
    bool getWest()
    {
        return (doors & bW) == bW;
    }
    bool getNorthExit()
    {
        return (doors & eN) == eN;
    }
    bool getEastExit()
    {
        return (doors & eE) == eE;
    }
    bool getSouthExit()
    {
        return (doors & eS) == eS;
    }
    bool getWestExit()
    {
        return (doors & eW) == eW;
    }

    void setNorth(bool set)
    {
        if (set)
            doors |= bN;
        else
            doors &= ~bN;
    }
    void setEast(bool set)
    {
        if (set)
            doors |= bE;
        else
            doors &= ~bE;
    }
    void setSouth(bool set)
    {
        if (set)
            doors |= bS;
        else
            doors &= ~bS;
    }
    void setWest(bool set)
    {
        if (set)
            doors |= bW;
        else
            doors &= ~bW;
    }
    void setNorthExit(bool set)
    {
        if (set)
            doors |= eN;
        else
            doors &= ~eN;
    }
    void setEastExit(bool set)
    {
        if (set)
            doors |= eE;
        else
            doors &= ~eE;
    }
    void setSouthExit(bool set)
    {
        if (set)
            doors |= eS;
        else
            doors &= ~eS;
    }
    void setWestExit(bool set)
    {
        if (set)
            doors |= eW;
        else
            doors &= ~eW;
    }
};

class Board
{
    // for posterity, world must be worldsize squared
    static const int WORLDSIZE = 5;
    static const int WORLD = 25;
    static const int MAXTURNS = 6;
    Room maze[WORLD];
    


public:
    int clientPos;
    int turns;

    Board() {
        clientPos = 12;
        turns = MAXTURNS;
        for (int i = 0; i < WORLD; i++)
        {
            maze[i].setWest(true);
            maze[i].setEast(true);

            if (i % 2 == 0)
                maze[i].setNorth(true);
            else
                maze[i].setSouth(true);

            maze[i].setNorthExit(isNorthBorder(i) && maze[i].getNorth());
            maze[i].setEastExit(isEastBorder(i) && maze[i].getEast());
            maze[i].setSouthExit(isSouthBorder(i) && maze[i].getSouth());
            maze[i].setWestExit(isWestBorder(i) && maze[i].getWest());
        }
    }
    ~Board()
    {

    }

    void resetBoard()
    {
        clientPos = 12;
        turns = MAXTURNS;
        for (int i = 0; i < WORLD; i++)
        {
            maze[i].setWest(true);
            maze[i].setEast(true);
            maze[i].setNorth(false);
            maze[i].setSouth(false);

            if (i % 2 == 0)
                maze[i].setNorth(true);
            else
                maze[i].setSouth(true);

            maze[i].setNorthExit(isNorthBorder(i) && maze[i].getNorth());
            maze[i].setEastExit(isEastBorder(i) && maze[i].getEast());
            maze[i].setSouthExit(isSouthBorder(i) && maze[i].getSouth());
            maze[i].setWestExit(isWestBorder(i) && maze[i].getWest());
        }
    }

    bool isNorthBorder(int ind)
    {
        if (ind >= 0 && ind < WORLDSIZE)
            return true;
        else
            return false;
    }

    bool isEastBorder(int ind)
    {
        if ((ind + 1) % WORLDSIZE == 0)
            return true;
        else
            return false;
    }

    bool isSouthBorder(int ind)
    {
        if (ind >= (WORLDSIZE * (WORLDSIZE - 1)) && ind < WORLD)
            return true;
        else
            return false;
    }

    bool isWestBorder(int ind)
    {
        if (ind % WORLDSIZE == 0)
            return true;
        else
            return false;
    }


    int getNorth(int room)
    {
        int otherRoom = room - WORLDSIZE;

        if (otherRoom < 0 || otherRoom >= WORLD)
            otherRoom = -1;

        return otherRoom;
    }
    int getEast(int room)
    {
        int otherRoom = room + 1;

        if (otherRoom < 0 || otherRoom >= WORLD)
            otherRoom = -1;

        return otherRoom;
    }
    int getSouth(int room)
    {
        int otherRoom = room + WORLDSIZE;

        if (otherRoom < 0 || otherRoom >= WORLD)
            otherRoom = -1;

        return otherRoom;
    }
    int getWest(int room)
    {
        int otherRoom = room - 1;

        if (otherRoom < 0 || otherRoom >= WORLD)
            otherRoom = -1;

        return otherRoom;
    }

    bool canMoveNorth(int room)
    {
        if (canExitNorth(room))
            return true;

        int oPos = getNorth(room);
        if (oPos == -1)
            return false;

        if (maze[room].getNorth() && maze[oPos].getSouth())
            return true;
        else
            return false;
    }

    bool canMoveEast(int room)
    {
        if (canExitEast(room))
            return true;

        int oPos = getEast(room);
        if (oPos == -1)
            return false;

        if (maze[room].getEast() && maze[oPos].getWest())
            return true;
        else
            return false;
    }

    bool canMoveSouth(int room)
    {
        if (canExitSouth(room))
            return true;

        int oPos = getSouth(room);
        if (oPos == -1)
            return false;

        if (maze[room].getSouth() && maze[oPos].getNorth())
            return true;
        else
            return false;
    }

    bool canMoveWest(int room)
    {
        if (canExitWest(room))
            return true;

        int oPos = getWest(room);
        if (oPos == -1)
            return false;

        if (maze[room].getWest() && maze[oPos].getEast())
            return true;
        else
            return false;
    }

    bool canExitNorth(int room)
    {
        return maze[room].getNorthExit();
    }

    bool canExitEast(int room)
    {
        return maze[room].getEastExit();
    }

    bool canExitSouth(int room)
    {
        return maze[room].getSouthExit();
    }

    bool canExitWest(int room)
    {
        return maze[room].getWestExit();
    }

    void rotate(int index, int steps)
    {
        bool up, right, down, left;

        for (int i = 0; i < steps; i++)
        {
            up = maze[index].getNorth();
            right = maze[index].getEast();
            down = maze[index].getSouth();
            left = maze[index].getWest();

            maze[index].setEast(up);
            maze[index].setSouth(right);
            maze[index].setWest(down);
            maze[index].setNorth(left);
        }

        maze[index].setNorthExit(isNorthBorder(index) && maze[index].getNorth());
        maze[index].setEastExit(isEastBorder(index) && maze[index].getEast());
        maze[index].setSouthExit(isSouthBorder(index) && maze[index].getSouth());
        maze[index].setWestExit(isWestBorder(index) && maze[index].getWest());
    }

    void SerializeMaze(char* buffer) {
        for (int i = 0; i < WORLD; i++)
            buffer[i] = maze[i].doors;
    }

    void DeserializeMaze(char* buffer) {
        for (int i = 0; i < WORLD; i++)
            maze[i].doors = buffer[i];
    }

    void SerializePosition(char* buffer) {
        for (int i = 0; i < WORLD; i++)
            copy_to_buffer(buffer, &clientPos, sizeof(int));
    }

    void DeserializePosition(char* buffer) {
        read_from_buffer(buffer, &clientPos);
    }

    char roomCenter(int pos)
    {
        if (pos == clientPos)
        {
            return PLAYER;
        }
        else
        {
            return PATH;
        }
    }

    char northPrint(int pos)
    {
        if (maze[pos].getNorth())
        {
            if (maze[pos].getNorthExit())
                return DOOR;
            else
                return PATH;
        }
        else
            return WALL;
    }

    char eastPrint(int pos)
    {
        if (maze[pos].getEast())
        {
            if (maze[pos].getEastExit())
                return DOOR;
            else
                return PATH;
        }
        else
            return WALL;
    }

    char southPrint(int pos)
    {
        if (maze[pos].getSouth())
        {
            if (maze[pos].getSouthExit())
                return DOOR;
            else
                return PATH;
        }
        else
            return WALL;
    }

    char westPrint(int pos)
    {
        if (maze[pos].getWest())
        {
            if (maze[pos].getWestExit())
                return DOOR;
            else
                return PATH;
        }
        else
            return WALL;
    }

    void printMaze()
    {
        for (int i = 0; i < WORLDSIZE; i++)
        {
            std::cout << WALL << northPrint((i * WORLDSIZE) + 0) << WALL << WALL << northPrint((i * WORLDSIZE) + 1) << WALL << WALL << northPrint((i * WORLDSIZE) + 2) << WALL << WALL << northPrint((i * WORLDSIZE) + 3) << WALL << WALL << northPrint((i * WORLDSIZE) + 4) << WALL << std::endl;
            std::cout << westPrint((i * WORLDSIZE) + 0) << roomCenter((i * WORLDSIZE) + 0) << eastPrint((i * WORLDSIZE) + 0) << westPrint((i * WORLDSIZE) + 1) << roomCenter((i * WORLDSIZE) + 1) << eastPrint((i * WORLDSIZE) + 1) << westPrint((i * WORLDSIZE) + 2) << roomCenter((i * WORLDSIZE) + 2) << eastPrint((i * WORLDSIZE) + 2) << westPrint((i * WORLDSIZE) + 3) << roomCenter((i * WORLDSIZE) + 3) << eastPrint((i * WORLDSIZE) + 3) << westPrint((i * WORLDSIZE) + 4) << roomCenter((i * WORLDSIZE) + 4) << eastPrint((i * WORLDSIZE) + 4) << std::endl;
            std::cout << WALL << southPrint((i * WORLDSIZE) + 0) << WALL << WALL << southPrint((i * WORLDSIZE) + 1) << WALL << WALL << southPrint((i * WORLDSIZE) + 2) << WALL << WALL << southPrint((i * WORLDSIZE) + 3) << WALL << WALL << southPrint((i * WORLDSIZE) + 4) << WALL << std::endl;
        }
    }

    void moveNorth()
    {
        if (canExitNorth(clientPos))
        {
            clientPos = WORLD;
            return;
        }
        if (canMoveNorth(clientPos))
        {
            clientPos = getNorth(clientPos);
        }
    }

    void moveEast()
    {
        if (canExitEast(clientPos))
        {
            clientPos = WORLD;
            return;
        }
        if (canMoveEast(clientPos))
        {
            clientPos = getEast(clientPos);
        }
    }

    void moveSouth()
    {
        if (canExitSouth(clientPos))
        {
            clientPos = WORLD;
            return;
        }
        if (canMoveSouth(clientPos))
        {
            clientPos = getSouth(clientPos);
        }
    }

    void moveWest()
    {
        if (canExitWest(clientPos))
        {
            clientPos = WORLD;
            return;
        }
        if (canMoveWest(clientPos))
        {
            clientPos = getWest(clientPos);
        }
    }

    int moveCursorNorth(int orPos)
    {
        if (orPos <= WORLDSIZE)
            return orPos;
        else
            return orPos - WORLDSIZE;
    }

    int moveCursorEast(int orPos)
    {
        if (orPos % 5 == 4)
            return orPos;
        else
            return orPos + 1;
    }

    int moveCursorSouth(int orPos)
    {
        if (orPos >= (WORLDSIZE - 1) * WORLDSIZE)
            return orPos;
        else
            return orPos + WORLDSIZE;
    }

    int moveCursorWest(int orPos)
    {
        if (orPos % 5 == 0)
            return orPos;
        else
            return orPos - 1;
    }

    int toggle(int i, bool up)
    {
        if (up)
        {
            if (i >= 3)
                return 3;
            else
                return i + 1;
        }
        else
        {
            if (i <= 1)
                return 1;
            else
                return i - 1;
        }
    }

    std::string coord(int i)
    {
        int halfWorld = (WORLDSIZE - 1) / 2;
        int xStat = (i % WORLDSIZE) - halfWorld;
        int yStat = (i - (i % WORLDSIZE)) / WORLDSIZE - halfWorld;
        std::stringstream ss;
        ss << "(" << xStat << ", " << yStat << ")";
        return ss.str();
    }

    std::string playing(bool isServer)
    {
        std::string st;
        if (isServer)
            st = "server";
        else
            st = "client";

        std::stringstream ss;

        ss << "current player: " << st << "      " << "turns left: " << turns;
        return ss.str();
    }

private:

};



int run_server();
int run_client();

const std::string WELCOME = "Would you like to play as client or server? ";
const std::string TYPEHERE = "Input their address here: ";
const std::string INVALID = "You inputted an invalid response. Try again here: ";
const double target_dt = 1.0 / 30.0;
const int TURNS = 10;

std::string link;

Board bort;

int main(int argc, char *argv[]) {
    bool isClient, correct = false;
    bool testing = false;
    std::string outputMessage;
    std::string input;
    std::string ip;

    SockLibInit();
    atexit(SockLibShutdown);

    outputMessage = WELCOME;


    while (!correct)
    {
        std::cout << "Welcome to Labrin" << std::endl << std::endl << outputMessage;

        std::cin >> input;

        if (input == "server" || input == "Server" || input == "SERVER" || input == "S" || input == "s")
        {
            isClient = false;
            correct = true;
        }
        else if (input == "client" || input == "Client" || input == "CLIENT" || input == "C" || input == "c")
        {
            isClient = true;
            correct = true;
        }
        else if (input == "t")
        {
            testing = true;
            correct = true;
        }
        else
        {
            outputMessage = INVALID;
        }

        system("cls");
    }

    if (!testing)
    {
        if (isClient)
        {
            outputMessage = TYPEHERE;
            correct = false;
            while (!correct)
            {
                std::cout << "Your partner will have received a look at their ip address. It will look like the following:" << std::endl << "IPv4 Address. . . . . . . . . . . : 100.10.98.4" << std::endl << outputMessage;

                std::cin >> input;

                ip = is_valid_ip_address(input);

                if (ip != "fail")
                {
                    correct = true;
                    link = input;
                    std::cout << "Connecting soon";
                }
                else
                {
                    outputMessage = INVALID;
                }
            }

            run_client();
        }
        else
        {
            system("C:\\Windows\\System32\\ipconfig");
            run_server();
        }
    }
    else
    {
        /*int dilidili = 0;
        double dee = 0;
        bool bork = false;
        std::cout << dilidili << std::endl;
        std::chrono::system_clock::time_point end;
        std::chrono::duration<double> delta;
        std::chrono::duration<double> jag = (std::chrono::duration<double>)target_dt;
        std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
        std::chrono::system_clock::time_point als = std::chrono::system_clock::now();
        while (!bork)
        {   
            system("cls");
            start = std::chrono::system_clock::now();

            dilidili++;
            dee += target_dt;
            std::cout << dee << std::endl;
            
            if (dilidili > 59)
                bork = true;

            end = std::chrono::system_clock::now();

            std::cout << (std::chrono::system_clock::now() - als).count() << std::endl;

            
            if (delta.count() < target_dt)
            {
                std::this_thread::sleep_for(jag - delta);
            }

           
        }*/

        using dsec = std::chrono::duration<double>;
        auto invFpsLimit = std::chrono::duration_cast<std::chrono::system_clock::duration>(dsec{ 1. / 30 });
        auto m_BeginFrame = std::chrono::system_clock::now();
        auto m_EndFrame = m_BeginFrame + invFpsLimit;
        unsigned frame_count_per_second = 0;
        std::chrono::system_clock::time_point als = std::chrono::system_clock::now();

        bool newInput = false;
        int yad = 12;
        int rotations = 1;
        auto prev_time_in_seconds = std::chrono::time_point_cast<std::chrono::seconds>(m_BeginFrame);
        bort.printMaze();
        std::cout << "tile: " << bort.coord(yad) << std::endl;
        std::cout << "rotations: " << rotations << std::endl;
        while (true)
        {
            if (newInput == true)
            {
                system("cls");
                bort.printMaze();
                std::cout << "tile: " << bort.coord(yad) << std::endl;
                std::cout << "rotations: " << rotations << std::endl;
            }

            newInput = false;

            if (GetAsyncKeyState(VK_LEFT))
            {
                yad = bort.moveCursorWest(yad);
                newInput = true;
            }
            else if (GetAsyncKeyState(VK_RIGHT))
            {
                yad = bort.moveCursorEast(yad);
                newInput = true;
            }
            else if (GetAsyncKeyState(VK_UP))
            {
                yad = bort.moveCursorNorth(yad);
                newInput = true;
            }
            else if (GetAsyncKeyState(VK_DOWN))
            {
                yad = bort.moveCursorSouth(yad);
                newInput = true;
            }

            if (GetAsyncKeyState(VK_PRIOR))
            {
                rotations = bort.toggle(rotations, false);
                newInput = true;
            }
            else if (GetAsyncKeyState(VK_NEXT))
            {
                rotations = bort.toggle(rotations, true);
                newInput = true;
            }

            if (GetAsyncKeyState(VK_SPACE))
            {
                bort.rotate(yad, rotations);
                newInput = true;
            }
            

            auto time_in_seconds = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
            ++frame_count_per_second;
            if (time_in_seconds > prev_time_in_seconds)
            {
                //std::cerr << frame_count_per_second << " frames per second\n";
                frame_count_per_second = 0;
                prev_time_in_seconds = time_in_seconds;
            }

            // This part keeps the frame rate.
            std::this_thread::sleep_until(m_EndFrame);
            m_BeginFrame = m_EndFrame;
            m_EndFrame = m_BeginFrame + invFpsLimit;
        }
    }
}

int run_server() {
    char buffer[4096];
    char inp[4096];
    bool quit = false;
    bool canPlay = false;
    bool hasReceivedMove = false;
    bool newInput = false;
    bool sent = false;
    bool serverLose = false;
    bool serverWin = false;
    bool waiting = false;
    bool hasPrintedWaiting = false;
    bool approveRestart = false;

    int yad = 12;
    int rotations = 1;

    Socket listen_sock(Socket::Family::INET, Socket::Type::STREAM);
    listen_sock.Bind(Address("0.0.0.0", 23500));
    listen_sock.Listen();

    Socket* conn_sock = new Socket();
    listen_sock.AcceptInto(*conn_sock);
    conn_sock->SetNonBlockingMode(true);

    using dsec = std::chrono::duration<double>;
    auto invFpsLimit = std::chrono::duration_cast<std::chrono::system_clock::duration>(dsec{ 1. / 30 });
    auto m_BeginFrame = std::chrono::system_clock::now();
    auto m_EndFrame = m_BeginFrame + invFpsLimit;
    unsigned frame_count_per_second = 0;
    std::chrono::system_clock::time_point als = std::chrono::system_clock::now();
    auto prev_time_in_seconds = std::chrono::time_point_cast<std::chrono::seconds>(m_BeginFrame);
    while (!quit)
    {
        

        if (approveRestart)
        {
            serverLose = false;
            serverWin = false;
            bort.resetBoard();
            system("cls");
            std::cout << bort.playing(false) << std::endl;
            bort.printMaze();
            buffer[0] = 'r';
            conn_sock->Send(buffer, sizeof(char));
            waiting = false;
            hasPrintedWaiting = false;
            approveRestart = false;
        }

        if (hasReceivedMove)
        {
            system("cls");
            std::cout << bort.playing(true) << std::endl;
            bort.printMaze();
            std::cout << "tile: " << bort.coord(yad) << std::endl;
            std::cout << "rotations: " << rotations << std::endl;
        }

        hasReceivedMove = false;

        if (newInput == true)
        {
            system("cls");
            std::cout << bort.playing(true) << std::endl;
            bort.printMaze();
            if (sent == false)
            {
                std::cout << "tile: " << bort.coord(yad) << std::endl;
                std::cout << "rotations clockwise: " << rotations << std::endl;
            }
        }

        if (sent == true)
        {
            bort.turns--;
            bort.SerializeMaze(buffer);
            // 25 because that's WORLD
            conn_sock->Send(buffer, 25);
            canPlay = false;
            system("cls");
            std::cout << bort.playing(true) << std::endl;
            bort.printMaze();
            if (bort.turns <= 0 && bort.clientPos < 25)
                serverWin = true;
        }

        if (serverLose)
        {
            std::cout << std::endl << "Server lost! awaiting response from client" << std::endl;
            buffer[0] = 'l';
            conn_sock->Send(buffer, sizeof(char));
            serverLose = false;
        }

        if (serverWin)
        {
            std::cout << std::endl << "Server won! awaiting response from client" << std::endl;
            buffer[0] = 'w';
            conn_sock->Send(buffer, sizeof(char));
            serverWin = false;
        }

        int nbytes_recvd = conn_sock->Recv(buffer, sizeof(buffer));
        if (nbytes_recvd == 0) {
            fprintf(stderr, "BLEHHH");
            quit = true;
        }
        else if (nbytes_recvd == -1)
        {
            if (conn_sock->GetLastError() == Socket::SOCKLIB_ETIMEDOUT)
            {
                //std::cout << "Received '" << "'\n";
            }
            else
            {
                quit = true;
            }
        }
        
        if (nbytes_recvd > 0)
        {
            if (nbytes_recvd == sizeof(char))
            {
                switch (buffer[0])
                {
                case 'a':
                    system("cls");
                    std::cout << bort.playing(false) << std::endl;
                    bort.printMaze();
                    break;
                case 'y':
                    std::cout << "Client wants restart" << std::endl;
                    std::cout << "Press y to restart, n to stop" << std::endl;
                    waiting = true;
                    break;
                case 'n':
                    quit = true;
                    break;
                default:
                    break;
                }
            }

            if (nbytes_recvd == sizeof(int))
            {
                //25 Because that's world
                bort.turns--;
                bort.DeserializePosition(buffer);
                if (bort.turns > 0)
                {
                    if (bort.clientPos < 25)
                    {
                        hasReceivedMove = true;
                        canPlay = true;
                    }
                    else
                    {
                        serverLose = true;
                    }
                }
                else
                {
                    serverWin = true;
                }
            }
        }



        newInput = false;
        sent = false;


        if (canPlay)
        {
            if (GetKeyState(VK_LEFT) & 0x80)
            {
                yad = bort.moveCursorWest(yad);
                newInput = true;
            }
            else if (GetKeyState(VK_RIGHT) & 0x80)
            {
                yad = bort.moveCursorEast(yad);
                newInput = true;
            }
            else if (GetKeyState(VK_UP) & 0x80)
            {
                yad = bort.moveCursorNorth(yad);
                newInput = true;
            }
            else if (GetKeyState(VK_DOWN) & 0x80)
            {
                yad = bort.moveCursorSouth(yad);
                newInput = true;
            }

            if (GetKeyState(VK_PRIOR) & 0x80)
            {
                rotations = bort.toggle(rotations, false);
                newInput = true;
            }
            else if (GetKeyState(VK_NEXT) & 0x80)
            {
                rotations = bort.toggle(rotations, true);
                newInput = true;
            }

            if (GetKeyState(VK_SPACE) & 0x80)
            {
                bort.rotate(yad, rotations);
                newInput = true;
                sent = true;
            }
        }

        if (waiting)
        {
            if (GetKeyState(0x59) & 0x80)
            {
                approveRestart = true;
            }
            else if (GetKeyState(0x4E) & 0x80)
            {
                quit = true;
            }
        }


        auto time_in_seconds = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
        ++frame_count_per_second;
        if (time_in_seconds > prev_time_in_seconds)
        {
            //std::cerr << frame_count_per_second << " frames per second\n";
            frame_count_per_second = 0;
            prev_time_in_seconds = time_in_seconds;
        }

        // This part keeps the frame rate.
        std::this_thread::sleep_until(m_EndFrame);
        m_BeginFrame = m_EndFrame;
        m_EndFrame = m_BeginFrame + invFpsLimit;
    }
    return 0;
}

int run_client() {
    char buffer[4096];
    char inp[4096];
    bool hasEntered = false;
    bool hasReceivedMove = false;
    bool canPlay = true;
    bool quit = false;
    bool sent = false;
    bool newInput = false;

    bool lost = false;
    bool waiting = false;
    bool hasPrintedWaiting = false;

    Socket sock(Socket::Family::INET, Socket::Type::STREAM);
    sock.Connect(Address(link, 23500));
    sock.SetNonBlockingMode(true);

    using dsec = std::chrono::duration<double>;
    auto invFpsLimit = std::chrono::duration_cast<std::chrono::system_clock::duration>(dsec{ 1. / 30 });
    auto m_BeginFrame = std::chrono::system_clock::now();
    auto m_EndFrame = m_BeginFrame + invFpsLimit;
    unsigned frame_count_per_second = 0;
    std::chrono::system_clock::time_point als = std::chrono::system_clock::now();
    auto prev_time_in_seconds = std::chrono::time_point_cast<std::chrono::seconds>(m_BeginFrame);
    system("cls");
    std::cout << bort.playing(false) << std::endl;
    bort.printMaze();
    while (!quit)
    {
        

        if (hasEntered == false)
        {
            buffer[0] = 'a';
            sock.Send(buffer, sizeof(char));
            hasEntered = true;
        }

        if (hasReceivedMove)
        {
            system("cls");
            std::cout << bort.playing(false) << std::endl;
            bort.printMaze();
        }

        hasReceivedMove = false;

        if (newInput == true)
        {
            bort.turns--;
            bort.SerializePosition(buffer);
            sock.Send(buffer, sizeof(int));
            canPlay = false;
            system("cls");
            std::cout << bort.playing(true) << std::endl;
            bort.printMaze();
        }

        int nbytes_recvd = sock.Recv(buffer, sizeof(buffer));
        if (nbytes_recvd == 0) {
            fprintf(stderr, "Server disconnected");
            quit = true;
        }
        else if (nbytes_recvd == -1)
        {
            if (sock.GetLastError() == Socket::SOCKLIB_ETIMEDOUT)
            {
                //std::cout << "Received null" << "'\n";
            }
            else
            {
                quit = true;
            }
        }
        //system("cls");
        if (nbytes_recvd > 0)
        {
            if (nbytes_recvd == 25)
            {
                bort.turns--;
                bort.DeserializeMaze(buffer);
                hasReceivedMove = true;
                canPlay = true;
            }
            
            if (nbytes_recvd == 1)
            {
                if (buffer[0] == 'w')
                {
                    canPlay = false;
                    std::cout << "Client lost" << std::endl;
                    std::cout << "Press y to restart, n to stop" << std::endl;
                    waiting = true;
                }
                else if (buffer[0] == 'l')
                {
                    canPlay = false;
                    std::cout << "Client won" << std::endl;
                    std::cout << "Press y to restart, n to stop" << std::endl;
                    waiting = true;
                }
                else if (buffer[0] == 'r')
                {
                    bort.resetBoard();
                    system("cls");
                    canPlay = true;
                    std::cout << bort.playing(false) << std::endl;
                    bort.printMaze();
                    waiting = false;
                    hasPrintedWaiting = false;
                }

            }
        }
        

        newInput = false;

        if (canPlay)
        {
            if (GetAsyncKeyState(VK_LEFT) && bort.canMoveWest(bort.clientPos))
            {
                bort.moveWest();
                newInput = true;
            }
            else if (GetAsyncKeyState(VK_RIGHT) && bort.canMoveEast(bort.clientPos))
            {
                bort.moveEast();
                newInput = true;
            }
            else if (GetAsyncKeyState(VK_UP) && bort.canMoveNorth(bort.clientPos))
            {
                bort.moveNorth();
                newInput = true;
            }
            else if (GetAsyncKeyState(VK_DOWN) && bort.canMoveSouth(bort.clientPos))
            {
                bort.moveSouth();
                newInput = true;
            }
        }
        
        if (waiting)
        {
            if (GetKeyState(0x59) & 0x80)
            {
                buffer[0] = 'y';
                sock.Send(buffer, sizeof(char));
                std::cout << std::endl << "Awaiting server" << std::endl;
            }
            else if (GetKeyState(0x4E) & 0x80)
            {
                buffer[0] = 'n';
                //sock.Send(buffer, sizeof(char));
                quit = true;
            }
        }


        auto time_in_seconds = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
        ++frame_count_per_second;
        if (time_in_seconds > prev_time_in_seconds)
        {
            //std::cerr << frame_count_per_second << " frames per second\n";
            frame_count_per_second = 0;
            prev_time_in_seconds = time_in_seconds;
        }

        // This part keeps the frame rate.
        std::this_thread::sleep_until(m_EndFrame);
        m_BeginFrame = m_EndFrame;
        m_EndFrame = m_BeginFrame + invFpsLimit;
    }

    return 0;
}