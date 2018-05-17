/**
@author Larchenko Roman
@email  LarchenkoRP@mail.ru
@date   01/05/2018
*/

#pragma once

#include <cstdlib>
#include <iostream>
#include <vector>
#include <functional>
#include <map>


#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "sha1.hpp"


//#include <Solver\SolverFactory.hpp>

using boost::asio::ip::udp;


//namespace Credits {
//	class ISolver;
//}

/**
@class SessionIO
����� ������������ ������ ������������ ������/�������� ���������� �� ��������� UDP.
*/
class SessionIO
{
public:
	/*!
	* \brief ����������� �� ���������.
	*/
	SessionIO();

	/*!
	* \brief ����������� ���������� �� ���������.
	*/
	virtual ~SessionIO();

	/*!
	* \brief ������� ������� �������.
	*/
	void Run();

private:

	unsigned int DefiningNode_;				/// ����������� ������ ���� ( ������� )
	const uint16_t version_ = 1;			/// ������ ����
	const char * host_port_ = { "9001" };	/// ���� ����� �� ���������
	const char * server_port_ = { "6000" }; /// ���� ����������� ������� �� ���������
	static bool OneReg;						/// ���� ����������� 

	SHA1 HashBlock_;
	
	/// ��������� ����
	struct PacketNode
	{
		std::string hash; /// ���
		std::string key;  /// ����
		std::string ip;	  /// IP �����
		std::string port; /// ���� 
	};

	std::string ServerHash_; /// ��� �������
	std::string ServerKey_;  /// ���� �������
	std::string MyHash_;     /// ��� ������� ����
	std::string MyPublicKey_;/// ��������� ���� ������� ����

	std::vector<std::string> �onfidantNodes_; /// ������ ���������� �����
	std::string GeneralNode_;                 /// ������� ����

	std::string calc_hash;
	//Credits::SolverFactory factory;
	//std::unique_ptr<Credits::ISolver> solver;


	/// ������������ ������� ����
	enum LevelNodes {
		Normal = 0,    /// ������� ����
		�onfidant , /// ���������� ����
		Main ,      /// ������� ����
		Write       /// ������� ����
	};

	/// ������������ ������
	enum CommandList {
		Registration = 1,         /// �����������
		UnRegistration ,		 /// �������������� 
		Redirect ,			 /// ����������
		GetHash ,              /// ������� ���
		SendHash ,		     /// ��������� ���
		SendTransaction ,      /// ��������� ����������
		GetTransaction ,       /// ������� ����������
		SendTransactionList ,  /// ��������� �������������� ����
		GetTransactionList ,   /// ������� �������������� ����
		SendVector ,           /// ��������� ������
		GetVector ,			 /// ������� ������
		SendMatrix ,           /// ��������� �������
		GetMatrix ,            /// ������� �������
		SendBlock ,            /// ��������� ���� ������
		GetHashAll ,          /// ������ ���(�) �� ���� �����
		SendIpTable ,         /// ��������� ������ ���������� ����� � �������� ���� �����
		SinhroPacket ,
		GiveHash2
	};

	/// ������������ ���������
	enum SubCommandList {
		RegistrationLevelNode = 1, /// ������� ������ ���������� � �������� ����
		GiveHash ,              /// ������ �� ���
		GetBlock ,			  /// ������ �� ���� ������
		GetBlocks,
		Empty
	};

	enum Version
	{
		version_1 = 1
	};

	enum { max_length = 62440, hash_length = 40, publicKey_length = 256 };

	/// ��������� ������ ������/�������� ����������
#pragma pack (push, 1)
	struct Packet
	{
		char	 command;                        /// �������
		char	 subcommand;						/// ����������
		char	 version;						/// ������
		char	 hash[hash_length];              /// ��� �����������/������������ ����
		char	 publicKey[publicKey_length];   /// ��������� ���� �����������/������������ ����
		char	 HashBlock[hash_length];			/// ��� �����
		uint16_t header;						/// ����� ���������
		uint16_t countHeader;					/// ���������� ����������
		char	 data[max_length];               /// ������
	};
#pragma pack (pop)
	Packet RecvBuffer;
	Packet SendBuffer;

	struct Storage
	{
		uint8_t HashBlock[hash_length];			 /// ��� �����
		uint16_t header;						 /// ����� ���������
		bool operator==(const Storage & param)
		{
			return !strncmp((const char*)this->HashBlock, (const char*)param.HashBlock, sizeof(HashBlock)) && 
				this->header == param.header;
		}
	};

	/*!
	* \brief ����� ������������ ���������� ����������.
	* \return ������ ��������� ���������� ������ ������/�������� ����������
	*/
	unsigned int �alcSumHeader() const;
	unsigned int SizePacketHeader;               /// ������ ��������� ���������� ������ ������/�������� ����������
	unsigned int SizePacketHeader2;

	boost::asio::io_service io_service_client_;  /// ������ �������
	boost::asio::io_service io_service_server_;  /// ������ �������
	
	udp::socket * InputServiceSocket_;           /// ����� ������� ���������� 
	udp::endpoint InputServiceRecvEndpoint_;	 /// ������� ����� ������ ����������
	udp::endpoint InputServiceSendEndpoint_;     /// ������� ����� �������� ���������� 
	udp::resolver InputServiceResolver_;         /// �������� �������

	udp::socket * OutputServiceSocket_;          /// ����� �������� ���������� 
	udp::endpoint OutputServiceRecvEndpoint_;    /// ������� ����� ������ ����������
	udp::endpoint OutputServiceSendEndpoint_;    /// ������� ����� ������ ����������
	udp::endpoint OutputServiceServerEndpoint_;  /// ������� ����� ����������� �������
	udp::resolver OutputServiceResolver_;		 /// �������� �������

	boost::circular_buffer<PacketNode> m_nodesRing;   /// ��������� ����� �������� �����
	boost::circular_buffer<Storage> BackData_;	     /// ��������� ����� �������� ���������� ����������

	//boost::detail::spinlock SpinLock_;               /// C���-����������
	boost::property_tree::ptree config;              /// ����� �������������
	//boost::asio::deadline_timer timer;				 /// ������


	std::string tmp_hash1;
	std::string tmp_hash2;
	std::map<unsigned int, std::string> blocks;

	/*!
	* \brief ����� ������������ ���������� ����������.
	*/
	void Initialization();

	void InitMap();

	/*!
	* \brief ����� ������ ����������.
	*/
	void InputServiceHandleReceive(const boost::system::error_code & error, std::size_t bytes_transferred);

	void outputHandleSend(boost::shared_ptr<std::string> message,
		 const boost::system::error_code& error,
		 std::size_t bytes_transferred);

	void inputHandleSend(boost::shared_ptr<std::string> message,
		 const boost::system::error_code& error,
		 std::size_t bytes_transferred);

	void SendBlocks(const char * buff, unsigned int size);

	void GetBlocks2(std::size_t bytes_transferred);



	/*!
	* \brief ����� ������ ����������.
	*/
	void outFrmPack(CommandList cmd, SubCommandList sub_cmd, Version ver,
					const char * data, unsigned int size_data);
	void outSendPack(unsigned int size_pck);

	void inFrmPack(CommandList cmd, SubCommandList sub_cmd, Version ver,
				   const char * data, unsigned int size_data);	
	void inSendPack(unsigned int size_pck);

	/*!
	* \brief ����� ������ ����������.
	*/
	void SolverSendData(char * buffer, unsigned int buf_size, char * ip_buffer, unsigned int ip_size, unsigned int cmd);

	/*!
	* \brief ����� ������ ���.
	*/
	void SolverSendHash(char * buffer, unsigned int buf_size, char * ip_buffer, unsigned int ip_size);

	/*!
	* \brief ����� ������� ��� ���� �����.
	*/
	void SolverGetHashAll();

	/*!
	* \brief ����� �������� ����������.
	*/
	void SolverSendTransaction(const char * data, unsigned size);

	/*!
	* \brief ����� �������� ����������.
	*/
	void SolverSendTransactionList(const char * data, unsigned size);

	/*!
	* \brief ����� �������� �������.
	*/
	void SolverSendVector(const char * data, unsigned size);

	/*!
	* \brief ����� �������� �������.
	*/
	void SolverSendMatrix(const char * data, unsigned size);

	/*!
	* \brief ����� �������� ����� ������.
	*/
	void SolverSendBlock(const char * data, unsigned size);

	/*!
	* \brief ����� ��������� ���������� � �������� ����.
	*/
	void GenTableRegistrationLevelNode( char * data, unsigned size);

	/*!
	* \brief ����� ��������� ���������� ���.
	*/
	void GenerationHash();

	/*!
	* \brief ����� ������� ���� ���������.
	*/
	void StartReceive();

	/*!
	* \brief ����� ��������� ���������� �����.
	*/
	bool RunRedirect(std::size_t bytes_transferred);
	
	/*!
	* \brief ����� �������������� �� ���������� �������.
	*/
	void InRegistrationNode();

	/*!
	* \brief ����� ����������� ���������� � �������� ����.
	*/
	void InRegistrationLevelNode(std::size_t bytes_transferred);

	/*!
	* \brief ����� ����������� �� ���������� �������.
	*/
	void RegistrationToServer();

	const std::string GenHashBlock(const char * buff, unsigned int size);

	void SendSinhroPacket();

	void DefiningNode(unsigned int init);
	
	std::function<void(char * buffer, unsigned int buf_size, char * ip_buffer, unsigned int ip_size, unsigned int cmd)> SolverSendData_;
};

