# include <QCoreApplication>
# include <QtNetwork>
# include <iostream>
# include <fstream>
# include <filesystem>
# include <string>
# include "sqlite3.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
      int i;
      for(i=0; i<argc; i++)
      {
            printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      }
      printf("\n");
      return 0;
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
                  std::cout << "Username login\n";
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
      std::cout << "Current path is " << std::filesystem::current_path() << '\n';
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
                  std::cout << "New connection; Output - ";
                  std::cout << SenderData.str() << '\n';
                  std::cout << "Sender adress is - " << Address << '\n';
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
                  case 103:
                  {
                        std::getline(SenderData, Token, '\0');
                        Path+="Files\\";Path+=Token;Path += ".txt";
                        File.open(Path, std::ios_base::in);
                        std::cout << std::boolalpha << File.is_open() << " - file state\n";
                        if (!File.is_open())
                        {
                              Checker = 0;
                              std::cout << "No such file\n";
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
                        std::cout << Queue.str().c_str() << " - sql query\n";
                        if (rc!=SQLITE_OK)
                        {
                              std::cout << "sqlite returened error\n";
                              tDatagram.setData(QByteArray::fromStdString("0"));
                              std::cout << ErrorMessage << '\n';

                        }
                        else
                        {
                              //std::cout << ErrorMessage << '\n';
                              tDatagram.setData(QByteArray::fromStdString("1" + *Results));
                        }
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
                  std::cout << "Done\n";
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
