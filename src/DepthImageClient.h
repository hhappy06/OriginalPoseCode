#pragma once
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

enum { DEP_IMAGE_SIZE = 640*480*2,
       RGB_IMAGE_SIZE = 640*480*3 };

class DepthImageClient
{
public:
    DepthImageClient()
        : m_ip_addr(""), m_port(0)
    {
    }
    ~DepthImageClient()
    {
    }
    void run(std::string & aip_addr, unsigned int aport = 12001)
    {
        assert(aport < 60000);
        m_port = aport;
        m_ip_addr = aip_addr;

        m_endpoint.address(boost::asio::ip::address::from_string(m_ip_addr)); //"172.16.88.163"
        m_endpoint.port(m_port);
    }
    bool get_depth_image(char * aimage_buff, size_t aimage_size)
    {
        bool res = false;

        if (!aimage_buff)
        {
            return res;
        }

        if (aimage_size == DEP_IMAGE_SIZE)
        {
            size_t frame_pos = 0;
            try
            {
                //boost::asio::io_service io_service;
                //tcp::endpoint endpoint(boost::asio::ip::address::from_string(m_ip_addr), m_port); //"172.16.88.163"
                boost::asio::ip::tcp::socket socket(m_io_service);
                socket.connect(m_endpoint);

                // send request
                boost::system::error_code ignored_error;
                boost::asio::write(socket, boost::asio::buffer("DEP_IMAGE"), ignored_error);

                frame_pos = 0;
                // receive one deopth frame
                for (;;)
                {
                    boost::system::error_code error;
                    size_t len = boost::asio::read(socket, boost::asio::buffer(aimage_buff, DEP_IMAGE_SIZE), error);
                    if (error == boost::asio::error::eof)
                    {
                        break; // Connection closed cleanly by peer. frame end
                    }
                    else if (error)
                        throw boost::system::system_error(error); // Some other error.

                    frame_pos += len;
                }
            }
            catch (std::exception& e)
            {
                std::cerr << e.what() << std::endl;
                return res;
            }

            if (frame_pos == DEP_IMAGE_SIZE)
            {
                //TRACE("DEP[100] = {%d}\n", ((unsigned short *)aimage_buff)[100]);
                res = true;
            }
        }
        return res;
    }
	bool get_rgb_image(char * aimage_buff, size_t aimage_size)
	{
		bool res = false;

		if (!aimage_buff)
		{
			return res;
		}

		if (aimage_size == RGB_IMAGE_SIZE)
		{
			size_t frame_pos = 0;
			try
			{
				//boost::asio::io_service io_service;
				//tcp::endpoint endpoint(boost::asio::ip::address::from_string(m_ip_addr), m_port); //"172.16.88.163"
				boost::asio::ip::tcp::socket socket(m_io_service);
				socket.connect(m_endpoint);

				// send request
				boost::system::error_code ignored_error;
				boost::asio::write(socket, boost::asio::buffer("RGB_IMAGE"), ignored_error);

				frame_pos = 0;
				// receive one deopth frame
				for (;;)
				{
					boost::system::error_code error;
					size_t len = boost::asio::read(socket, boost::asio::buffer(aimage_buff, RGB_IMAGE_SIZE), error);
					if (error == boost::asio::error::eof)
					{
						break; // Connection closed cleanly by peer. frame end
					}
					else if (error)
						throw boost::system::system_error(error); // Some other error.

					frame_pos += len;
				}
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << std::endl;
				return res;
			}

			if (frame_pos == RGB_IMAGE_SIZE)
			{
				//TRACE("DEP[100] = {%d}\n", ((unsigned short *)aimage_buff)[100]);
				res = true;
			}
		}
		return res;
	}
    
private:
    boost::asio::io_service m_io_service;
    boost::asio::ip::tcp::endpoint m_endpoint;
    unsigned int m_port;
    std::string  m_ip_addr;
};
