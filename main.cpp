# include <QCoreApplication>
# include <QtNetwork>
# include <iostream>
# include <fstream>
# include <filesystem>
# include <string>
# include "sqlite3.h"
#include <QDir>
#include <QUrl>
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
      int i;
      for(i=0; i<argc; i++)
      {
            std::cout << "Index is - " << i << "\nColumn name is - " << azColName[i]
                      << "\n Argument is - " << (argv[i] ? argv[i] : "NULL") << '\n';
      }
      std::cout << "\nENDED PRITNING\n";
      std::cout.flush();
      return 0;
}

static signed int SubjectCallback(void* SaveResults, signed int Counter, char** Results, char** Columns)
{
      std::string* lSaveResults {static_cast<std::string*>(SaveResults)};
      if (Counter==0)
      {
            return 0;
      }
      else
      {
            std::stringstream ResultsBuffer{lSaveResults->c_str()};

            std::cout.flush();
            std::string Delims{"/"},
                         Group{};
            char* Buffer{std::strtok(Results[2], Delims.c_str())};
            std::getline(ResultsBuffer, Group, '\n');
            bool IsAllowed{0};
            for(;Buffer;)
            {
                  if (Buffer == Group)
                  {
                        IsAllowed = 1;
                  }
                  Buffer=std::strtok(nullptr, Delims.c_str());
            }
            if (IsAllowed)
            {
                  lSaveResults->append("\n");
                  lSaveResults->append(Results[1]);
            }
            return 0;
      }
}

static signed int GroupCall(void* SaveResults, signed int Counter, char** Results, char** Columns)
{
      std::string* lSaveResults {static_cast<std::string*>(SaveResults)};
      if (Counter==0)
      {
            return 0;
      }
      else
      {
            lSaveResults->append(Results[5]);
            return 0;
      }
}

static signed int SqlCallback(void* SaveResults, signed int Counter, char** Results, char** Columns)
{
      std::string* lSaveResults {static_cast<std::string*>(SaveResults)};
      if (Counter==0)
      {
            return 1;
      }
      else
      {
            if (*Results[4]=='1')
            {
                  lSaveResults->append("1");
            }
            else
            {
                  std::cout << *Results[4] << '\n';
                  std::cout << "Пользователь зашел\n";
            }            
            lSaveResults->append("\n");
            lSaveResults->append(Results[3]);
            lSaveResults->append("\n");
            lSaveResults->append(Results[5]);
            return 0;
      }
}

void ProcessConenctions(QUdpSocket* Socket)
{
      unsigned int DataSize{200};
      char Data[DataSize];
      std::cout << "Путь до exe " << std::filesystem::current_path() << '\n';
      for(;;)
      {
            if (Socket->hasPendingDatagrams() == true)
            {
                  std::fstream File{};
                  bool Checker{1};
                  auto tDatagram{Socket->receiveDatagram()};
                  std::string Address{tDatagram.senderAddress().toString().toStdString()};
                  signed int Port{tDatagram.senderPort()};
                  std::stringstream SenderData{};
                  SenderData << tDatagram.data().toStdString();                  
                  std::cout << "Новое подключение по адресу " << Address << '\n';
                  std::cout.flush();
                  char RequestType{};
                  std::string Token;
                  std::getline(SenderData, Token, '\n');
                  RequestType = Token[0];

                  std::string Path{};
                  tDatagram.clear();
                  if (RequestType=='\0')
                  {
                        RequestType = '0';
                  }
                  switch (RequestType)
                  {
                  case 97:
                  {
                        std::cout << "Файл посещения прислан\n";
                        continue;
                        break;
                  }
                  case 103:
                  {
                        std::getline(SenderData, Token, '\0');
                        Path+="Files\\";Path+=Token;Path += ".txt";
                        File.open(Path, std::ios_base::in);
                        std::cout << std::boolalpha << File.is_open() << " - Состояние файла\n";
                        if (!File.is_open())
                        {
                              Checker = 0;
                              std::cout << "Такого файла нет\n";
                        }
                        break;
                  }
                  case 116:
                  {
                        std::getline(SenderData, Token, '\0');
                        File.open(Path, std::ios_base::in);
                        Path+="Files\\";Path+=Token;Path += ".txt";
                        std::cout << std::boolalpha << File.is_open() << " - file state\n";
                        if (!File.is_open())
                        {
                              Checker = 0;
                              std::cout << "No such file\n";
                        }
                        break;
                  }
                  case 108:
                  //Database call
                  {
                        Checker = 0;
                        std::string Login{};
                        std::string Password{};
                        std::stringstream Queue;
                        std::getline(SenderData, Login, ' ');
                        std::getline(SenderData, Password, '\0');
                        Queue << "select * from Students where Login = '" << Login
                              << "' and Password = '" << Password << "';";
                        sqlite3* Database;
                        auto rc = sqlite3_open("Files\\Diploma.sqlite", &Database);
                        if (rc)
                        {
                              fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(Database));
                              sqlite3_close(Database);
                              return;
                        }
                        std::string* Results = new std::string();
                        char* ErrorMessage{};
                        rc = sqlite3_exec(Database, Queue.str().c_str(), SqlCallback, Results, &ErrorMessage);
                        //std::cout << Queue.str().c_str() << " - sql query\n";
                        if (rc!=SQLITE_OK || *Results == "")
                        {
                              std::cout << "Ошибка логина\n";
                              tDatagram.setData(QByteArray::fromStdString("0"));
                              std::cout << ErrorMessage << '\n';
                        }
                        else
                        {
                              std::cout << "Успешный логин от - " <<  *Results << '\n';
                              tDatagram.setData(QByteArray::fromStdString("1" + *Results));
                        }
                        sqlite3_close(Database);
                        break;
                  }
                  case 102:
                  { //subject files files
                        Checker = 0;
                        std::string SubjectName{}, FileName{};
                        std::getline(SenderData, SubjectName, '\n');
                        std::getline(SenderData, FileName, '\n');
                        std::stringstream Path;
                        Path << "test/" << FileName;
                        std::cout << Path.str().c_str() << '\n';

                        QFile File(Path.str().c_str());
                        if (!File.exists())
                        {
                              tDatagram.setData(QByteArray::fromStdString("0")) ;
                              std::cout << "File is nto open\n";
                              break;
                        }
                        else
                        {
                              if (!File.open(QIODevice::ReadOnly | QIODevice::Text))
                              {

                                    tDatagram.setData(QByteArray::fromStdString("0")) ;
                                    std::cout << "File is not open\n";
                                    break;
                              }
                        }
                        tDatagram.setData(File.readAll());
                        File.close();
                        break;
                  }
                  case 115:
                  { //subject files
                        Checker = 0;
                        std::string SubjectName{}, UserName{};
                        std::getline(SenderData, SubjectName, '\n');
                        std::getline(SenderData, UserName, '\n');
                        std::stringstream Queue;
                        Queue.clear();
                        Queue << "select * from Students where FIO = '" << UserName << "';";
                        sqlite3* Database;
                        auto rc = sqlite3_open("Files\\Diploma.sqlite", &Database);
                        if (rc)
                        {
                              fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(Database));
                              sqlite3_close(Database);
                              return;
                        }
                        std::string* Results = new std::string();
                        char* ErrorMessage{};
                        rc = sqlite3_exec(Database, Queue.str().c_str(), GroupCall, Results, &ErrorMessage);
                        //std::cout << " - sql query\n";
                        assert(!Results->empty());
                        //Queue.clear();
                        Queue.str(std::string());
                        Queue << "select * from Files where Subject = '" << SubjectName
                              << "';";
                        std::cout.flush();
                        rc = sqlite3_exec(Database, Queue.str().c_str(), SubjectCallback, Results, &ErrorMessage);
                        if (rc!=SQLITE_OK || *Results == "")
                        {
                              std::cout << "sqlite returened error\n";
                              tDatagram.setData(QByteArray::fromStdString("0"));
                              std::cout << ErrorMessage << '\n';
                        }
                        else
                        {
                              std::cout << "Отправлены файлы дисциплины" << '\n';
                              const char* Delims{"\n"};
                              std::string ToSent{};
                              char* ToCut{const_cast<char*>(Results->c_str())};
                              char* Token{std::strtok(ToCut, Delims)};
                              Token = std::strtok(nullptr, Delims);
                              for(;Token;)
                              {
                                    ToSent.append(Token);
                                    ToSent.append("\n");
                                    Token = std::strtok(nullptr, Delims);
                              }
                              tDatagram.setData(QByteArray::fromStdString(ToSent));
                        }
                        sqlite3_close(Database);
                        break;
                  }
                  default:
                  {
                        Checker = 0;
                        std::cout << "No such Request\n";
                        break;
                  }
                  }
                  if (Checker)
                  {
                        std::stringstream FileContent{};
                        FileContent << File.rdbuf();
                        tDatagram.setData(QByteArray::fromStdString(FileContent.str()));
                        //std::cout << File.rdbuf() << '\n'; //ИДИОТ!!! КАРЕТКА В КОНЦЕ.
                        std::cout.flush();
                  }
                  //std::cout.flush();
                  tDatagram.setDestination(QHostAddress(Address.c_str()), Port);
                  Socket->writeDatagram(tDatagram);
                  std::cout << "Пакет отправлен\n";
                  File.close();
                  std::cout.flush();
            }
      }
}

int main(int argc, char *argv[])
{

      QUdpSocket *MainSocket = new QUdpSocket;
      MainSocket->bind(QHostAddress::LocalHost, 32323);
      ProcessConenctions(MainSocket);
      // sqlite3* Database;
      // if (sqlite3_open("Files\\Diploma.sqlite", &Database))
      // {
      //       std::cout << "Cant open database\n";
      //       return 1;
      // }
      // char* ErrorText = 0;
      // auto Error = sqlite3_exec(Database, "select * from Students where Login = 'email@email.com';", callback, 0, &ErrorText);
      // if ( Error != SQLITE_OK)
      // {
      //       std::cout << ErrorText;
      // }
      // sqlite3_close(Database);
      return 0;
}
