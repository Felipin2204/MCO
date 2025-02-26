/*
 * tcpclient.cpp
 *
 *  Created on: 19 feb. 2021
 *      Author: Juan Aznar Poveda
 */

#include "tcpclient.h"

#include "flatbuffers/flatbuffers.h"
#include "../flatbuffers/Reply_generated.h"
#include "../flatbuffers/Request_generated.h"
#include "../flatbuffers/Header_generated.h"




TcpClient::TcpClient() : s(io_context)
{

}




TcpClient::~TcpClient() {
    HeaderT header;
          header.size=1;

          header.type=2; //To close
          flatbuffers::FlatBufferBuilder header_builder;
          header_builder.Finish(Header::Pack(header_builder, &header));
          auto  header_size=header_builder.GetSize();
          auto header_data=header_builder.GetBufferPointer();
          boost::asio::write(s, boost::asio::buffer(header_data, header_size));

}

/*
    Connect to a host on a certain port number
 */
bool TcpClient::conn(std::string address , int port)
{
    try {
        boost::asio::ip::tcp::resolver resolver(io_context);
        std::cout<<"Trying to connect to CVX server"<<std::endl;
        boost::asio::connect(s, resolver.resolve(address, std::to_string(port)));
        std::cout<<"Connected to CVX server"<<std::endl;
    } catch (std::exception& e)
    {
        throw omnetpp::cRuntimeError( e.what());
    }
    return true;
}

std::vector<double>* TcpClient::request(int id,std::vector<double> &omega,  std::vector<double>& channelWeights, std::vector<double>& tmin,
        std::vector<double> &sumWeights, std::vector<double> &capacity,
        double d, double capacity_max, double alpha, double nv, bool regularized, double beta) {
    try {
        RequestT req;
        req.id=id;
        req.lambda=channelWeights;
        req.sum_weights=sumWeights;
        req.omega=omega;
        req.tmin=tmin;
        req.capacity=capacity;
        req.d=d;
        req.max_capacity=capacity_max;
        req.alpha=alpha;
        req.nv=nv;
        req.regularized=regularized;
        req.beta=beta;

        flatbuffers::FlatBufferBuilder fbb;
        fbb.Finish(Request::Pack(fbb, &req));
        auto size=fbb.GetSize();
        auto data=fbb.GetBufferPointer();


        //Header size as per current format is 24 bytes. Unless we use 0 for the type. Actually, we only have reply/request so we do
        //not really need type. So we keep numbers above 0
        HeaderT header;
        header.size=size;

        header.type=1;
        flatbuffers::FlatBufferBuilder header_builder;
        header_builder.Finish(Header::Pack(header_builder, &header));
        auto  header_size=header_builder.GetSize();
        auto header_data=header_builder.GetBufferPointer();
        //std::cout<<"Header size:"<<header_size<<"; Request size: "<<size<<std::endl;

        boost::asio::write(s, boost::asio::buffer(header_data, header_size));
        boost::asio::write(s, boost::asio::buffer(data, size));


        //Now read reply.
        auto reply_header = new std::vector<uint8_t>(header_size);
        //This code below does not seem to work...why?
        //auto reply_header = new std::vector<uint8_t>(header_size);
        //size_t reply_length = boost::asio::read(s,
          //      boost::asio::buffer(reply_header, 24));

        boost::system::error_code error;
        auto r=s.read_some(boost::asio::buffer(*reply_header),error);
               //Check
        //std::cout<<"Reply. Read bytes="<<reply_length<<std::endl;
        //for (auto b : *reply_header) {
          //      printf("%.2X\n",b);
        //}
        if (r==0) {
            throw omnetpp::cRuntimeError( error.value());
        }
        auto reply_header_data=flatbuffers::GetRoot<Header>(reply_header->data());
        auto reply_size=reply_header_data->size();
        //std::cout<<"Reply size="<<reply_size<<std::endl;
        auto reply_data=new std::vector<uint8_t>(reply_size);
       // size_t  reply_length = boost::asio::read(s,
               // boost::asio::buffer(reply_data, reply_size));

        r=s.read_some(boost::asio::buffer(*reply_data),error);
        if (r==0) {
                    throw omnetpp::cRuntimeError( error.value());
        }
        ReplyT reply;
        flatbuffers::GetRoot<Reply>(reply_data->data())->UnPackTo(&reply);
        if (reply.status==1) {
            delete reply_header;
            //TODO: is this efficient?
            auto traffic = new std::vector<double>(reply.r);
            delete reply_data;
            ;        return traffic;
        } else {
            delete reply_header;
            delete reply_data;
            return nullptr;
        }
    }  catch (std::exception& e)
    {
        throw omnetpp::cRuntimeError( e.what());
    }

}


