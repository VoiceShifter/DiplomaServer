# include <QCoreApplication>
# include <QtNetwork>
# include <iostream>
# include <fstream>
# include <filesystem>
# include <string>
# include <QtSql/QSqlDatabase>
# include <QtSql/QSqlDriver>
# include <QtSql/QSqlQuery>
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
                  std::getline(SenderData, Token, '\0');
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
                              std::string Login{std::strtok(const_cast<char*>(Token.c_str()), " ")};
                              std::string Password{std::strtok(nullptr, "\0 \n")};
                              QSqlDatabase Students{QSqlDatabase::addDatabase("QSQLITE")};
                              Students.setDatabaseName("Dimploma.sqlite");
                              std::stringstream Queue;
                              Queue << "select * from Students where Login = " << Login
                                    << "and Password = " << Password << ";";
                              auto QueueReturn{Students.exec(Queue.str().c_str())};
                              if (QueueReturn.isValid())
                              {
                                    tDatagram.setData(QByteArray::fromStdString("1"));
                                    Checker = 0;
                              }
                              else
                              {
                                    tDatagram.setData(QByteArray::fromStdString("0"));
                                    Checker = 0;
                              }
                              break;
                        }
                  default:
                  {
                        std::cout << "No such Request\n";
                        Checker = 0;
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
            }
      }
}

int main(int argc, char *argv[])
{
      QUdpSocket *MainSocket = new QUdpSocket;
      MainSocket->bind(QHostAddress::LocalHost, 32323);
      ProcessConenctions(MainSocket);
      return 0;
}
