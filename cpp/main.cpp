#include "test.pb.h"
#include <fstream>
#include <iostream>
#include <libpq-fe.h>
#include <exception>
#include <arpa/inet.h>

#define DBHOST std::string("127.0.0.1")
#define DBPORT 5432
#define DBNAME std::string("test_proto")
#define DBUSER std::string("postgres")
#define DBPASS std::string("postgres")

#define SIZE_INT 4
#define SIZE_LONG 8
#define SIZE_SHORT 2
#define SIZE_DOUBLE 8

//void generateMessage();
//int binaryCopy(PGconn* conn, void* buffer, int size);
int addResultSerialized(PGconn* conn, long simId, std::string agentId, long agentTypeId, double simTime, void* data, int data_size, bool isFirst, bool isLast);
//int sendResultsToDB(PGconn* c, void* data, long size);
int putHeader(PGconn* conn);
int putTrailer(PGconn* conn);
void generateMessage();

class Exception : public std::exception{
private:
	std::string info;
public:
	Exception(std::string _info){
        info = _info;
    }

    Exception(const char* _info){
        info = std::string(_info);
    }

	virtual const char* what() const throw(){
        return info.data();
    }
};


int main(){
    try{
        generateMessage();
    }catch(Exception e){
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

void checkError(PGconn* conn, int res){
    if(res == 0){
        std::cout <<"Could not queue data";
        throw Exception("could not queue data");
    }
    if(res == -1){
        std::cout << "ERROR:" << std::endl;
        std::cout << PQerrorMessage(conn) << std::endl;
        throw Exception(PQerrorMessage(conn));
    }
}
void sendToDB(void* data, int size){
    //create connection
    PGconn* conn = PQconnectdb(("dbname="+DBNAME+" user="+DBUSER+" password="+DBPASS+" hostaddr="+DBHOST+" port="+ std::to_string(DBPORT)).c_str());
    if(PQstatus(conn) != CONNECTION_OK){
        throw Exception("Connection to DB failed");
    }

    //get into COPY mode
    PGresult* r;
    r = PQexec(conn, "COPY results_serialized FROM STDIN (FORMAT BINARY)");
    if(PQresultStatus(r) != PGRES_COPY_IN)
        throw Exception(PQerrorMessage(conn));
    PQclear(r);

    checkError(conn, putHeader(conn));
    checkError(conn, addResultSerialized(conn, 5, "Agent 1", 9223372036854775805, 7.28, data, size, false,false));
    checkError(conn, addResultSerialized(conn, 1, "Agent 245", 05, 1.21, data, size, false,false));
    checkError(conn, putTrailer(conn));

    //end copy
   int  res = PQputCopyEnd(conn,nullptr);
    std::cout << PQerrorMessage(conn) << std::endl;
	if(res == -1){
		throw Exception(PQerrorMessage(conn));
	}
	if(res == 0){
		throw Exception("Could not queue end of copy");
	}
}

void generateMessage(){
    TestMessage test;

    test.set_input(2.3456);
    test.set_output(5.4321);
    test.set_info(5.0);

    int size = test.ByteSize();
    void* buffer = malloc(size);
    test.SerializeToArray(buffer,size);


    sendToDB(buffer, size);
    free(buffer);
}


// int binaryCopy(PGconn* conn, void* data, int size){
//     //convert to network order
//     //toNetworkOrder((char*)data, size);
    
//     std::string header      = std::string("PGCOPY\n\377\r\n");
//     int nullbyte            = 0;
//     int flags               = htonl(0);
//     int header_extionsion   = htonl(0);
//     short number_fields     = htons(1);
//     int   size_field        = htonl(size);
//     short trailer           = htons(-1);

//     int size_buffer = header.size()+1+4+4+2+4+size+2;
//     void* buffer = malloc(size_buffer);

//     int offset = 0;
//     memcpy(buffer+offset, (void*) header.data(), header.size());
//     std::cout << "Header size: " << header.size() << std::endl;
//     offset+=header.size();
//     memcpy(buffer+offset, (void*) &nullbyte, 1);
//     offset+=1;
//     memcpy(buffer+offset, (void*) &flags,4);
//     offset+=4;
//     memcpy(buffer+offset, (void*) &header_extionsion,4);
//     offset+=4;
//     memcpy(buffer+offset, (void*) &number_fields, 2);
//     offset+=2;
//     memcpy(buffer+offset, (void*) &size_field, 4);
//     offset+=4;
//     memcpy(buffer+offset, (void*) data, size);
//     offset+=size;
//     memcpy(buffer+offset, (void*) &trailer, 2);
//     offset+=2;

//     std::ofstream file;
//     std::ofstream file_only;

//     file.open("buffer");
//     file_only.open("buffer_content");

//     file.write((const char*) buffer, size_buffer);
//     file_only.write((const char*) data, size);

//     file_only.close();
//     file.close();

//     return PQputCopyData(conn, (const char*)buffer, size_buffer);
// }

double htond(double d);
long htonll(long l);


int putHeader(PGconn* conn){
    char* header            = "PGCOPY\n\377\r\n";              //10
    int nullbyte            = 0;                               //1
    int flags               = htonl(0);                        // SIZE_INT
    int header_extionsion   = htonl(0);                        // SIZE_INT

    //calc full size for buffer(header, zero byte, flags, header_extension, trailer)
    int size_buffer = 10 + 1 + SIZE_INT + SIZE_INT;

    //allocate
    void* buffer = malloc(size_buffer);

    //copy everything into the buffer

    //header
    int offset = 0;
    memcpy(buffer+offset, (void*) header, 10);
    offset+=10;
    memcpy(buffer+offset, (void*) &nullbyte, 1);
    offset+=1;
    memcpy(buffer+offset, (void*) &flags,4);
    offset+=4;
    memcpy(buffer+offset, (void*) &header_extionsion,4);
    offset+=4;
    return PQputCopyData(conn, (const char*) buffer, size_buffer);
}

int addResultSerialized(PGconn* conn, long simId, std::string agentId, long agentTypeId, double simTime, void* data, int data_size, bool isFirst, bool isLast){
    //create header
    char* header            = "PGCOPY\n\377\r\n";              //10
    int nullbyte            = 0;                               //1
    int flags               = htonl(0);                        // SIZE_INT
    int header_extionsion   = htonl(0);                        // SIZE_INT
    short trailer           = htons(-1);                      //SIZE_SHORT
    short number_fields     = htons(5);                       //SIZE_SHORT


    int buffer_size = 0;
    if(isFirst){
        //calc full size for buffer(header, zero byte, flags, header_extension, trailer)
        buffer_size+=10 + 1 + SIZE_INT + SIZE_INT;
    }
    if(isLast){
        buffer_size+=SIZE_SHORT;
    }
    

    


    buffer_size += SIZE_SHORT;                // for tuple (number of fields)
    buffer_size+= SIZE_INT + SIZE_LONG;      // size_field + simid
    buffer_size+= SIZE_INT + agentId.size(); // size_field + agentid
    buffer_size+= SIZE_INT + SIZE_LONG;      // size_field + agenttypeid
    buffer_size+= SIZE_INT + SIZE_DOUBLE;    // size_field + simtime
    buffer_size+= SIZE_INT + data_size;      // size_field + data

    int offset = 0;
    void* buffer = malloc(buffer_size);

    if(isFirst){
        //copy header
        memcpy(buffer+offset, (void*) header, 10);
        offset+=10;
        memcpy(buffer+offset, (void*) &nullbyte, 1);
        offset+=1;
        memcpy(buffer+offset, (void*) &flags,4);
        offset+=4;
        memcpy(buffer+offset, (void*) &header_extionsion,4);
        offset+=4;
    }

    //set number of fields per tuple
    memcpy(buffer+offset, (void*) &number_fields, 2);
    offset+=2;

    int size_int    = htonl(SIZE_INT);
    int size_double = htonl(SIZE_DOUBLE);
    int size_long   = htonl(SIZE_LONG);
    int size_short  = htonl(SIZE_SHORT);
  
    //simid
    memcpy(buffer+offset, (void*) &size_long, SIZE_INT);
    offset+=SIZE_INT;
    long simId_n = htonll(simId);
    memcpy(buffer+offset, (void*) &simId_n, SIZE_LONG);
    offset+=SIZE_LONG;

    //agentid
    int agentid_size = htonl(agentId.size());
    memcpy(buffer+offset, (void*) &agentid_size, SIZE_INT);
    offset+=SIZE_INT;
    memcpy(buffer+offset, (void*) agentId.data(), agentId.size());
    offset+=agentId.size();

    //agenttypeid
    memcpy(buffer+offset, (void*) &size_long, SIZE_INT);
    offset+=SIZE_INT;
    long agentTypeId_n = htonll(agentTypeId);
    memcpy(buffer+offset, (void*) &agentTypeId_n, SIZE_LONG);
    offset+=SIZE_LONG;

    //simtime
    memcpy(buffer+offset, (void*) &size_double, SIZE_INT);
    offset+=SIZE_INT;
    double simTime_n = htond(simTime);
    memcpy(buffer+offset, (void*) &simTime_n, SIZE_DOUBLE);
    offset+=SIZE_DOUBLE;

    //data
    int data_size_network = htonl(data_size);
    std::cout << data_size << std::endl;
    memcpy(buffer+offset, (void*) &data_size_network, SIZE_INT);
    offset+=SIZE_INT;
    memcpy(buffer+offset, data, data_size);
    offset+=data_size;

    //trailer
    if(isLast){
        memcpy(buffer+offset, (void*) &trailer, 2);
        offset+=2;
    }

    std::cout << "Buffer size: "<<buffer_size << std::endl;
    std::ofstream file;
    file.open("buffer");
    file.write((const char*) buffer, buffer_size);
    file.close();

    return PQputCopyData(conn, (const char*) buffer, buffer_size);
}

int putTrailer(PGconn* conn){
     short trailer = htons(-1);
     return PQputCopyData(conn, (const char*) &trailer, 2);
}

long htonll(long l){
    char* p = (char *) &l;
    char* l_n = (char *) malloc(SIZE_DOUBLE);
    for(int i=0;i<SIZE_DOUBLE;i++){
        l_n[i] = p[SIZE_DOUBLE-i-1];
    }
    long ret = *((long *) l_n);
    free(l_n);
    return ret;
}

double htond(double d){
    char* p = (char *) &d;
    char* c = (char *) malloc(SIZE_DOUBLE);
    for(int i=0;i<SIZE_DOUBLE;i++){
        c[i] = p[SIZE_DOUBLE-i-1];
    }
    double ret = *((double*) c);
    free(c);
    return ret;
}