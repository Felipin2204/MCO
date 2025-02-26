/*
 * tcpclient.h
 *
 *  Created on: 19 feb. 2021
 *      Author: Juan Aznar Poveda
 */

#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_
/*
    C++ Socket Client
 */
#include <omnetpp.h>
#include <boost/asio.hpp>







/*
    TCP Client class
 */
class TcpClient
{
protected:
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket s;


public:
    virtual ~TcpClient();
    TcpClient();
    bool conn(std::string address, int port);
    virtual  std::vector<double>* request(int id, std::vector<double>& omega, std::vector<double>& channelWeights, std::vector<double>& tmin, std::vector<double>& sumWeights, std::vector<double>& capacity, double d, double capacity_max, double alpha, double nv, bool regularized, double beta);



};



#endif /* TCPCLIENT_H_ */


