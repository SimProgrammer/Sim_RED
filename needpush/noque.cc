#include <string.h>
#include "queue.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include "random.h"
using namespace std;

class NoQue;
class NoQue : public Queue {
public :
	NoQue(string s);
	~NoQue();

protected :
	virtual int command(int argc, const char*const* argv);
	virtual Packet *deque(void);
	virtual void enque(Packet *pkt);

	PacketQueue *q_ = new PacketQueue(); //包队列(缓冲区)

	void recordPktcnt_allflowpertd();
	void recordTp_gd();
	void recordTd_gd();
	int sid;   //链路的发出端id
	int did;   //链路接收端id
	string path;
	double allflow_td[3];   //所有流端到端时延
	int allflow_td_pkt[3];   //所有流端到端时延,统计包数
	int countTp_gd;   //统计地面端吞吐量包数
	double pertime_;
	double wholetime;
	double countTd_gd;   //时延统计
	double countTdpkt_gd;   //时延包数统计
	int testpkt;   //测试统计用


	int isSat_;   //是否为卫星网络环境

};

//在OTCL里面注册我的NoQue类
static class NoQueClass : public TclClass {
public :
	NoQueClass() : TclClass("Queue/NoQue") {}
	TclObject *create(int argc, const char*const* argv) {
		string s = argv[4];
		return (new NoQue(s));
	}
} class_tque;

NoQue::NoQue(string s) {
	//获取链路的两端
	char buf[10];
	strcpy(buf, s.c_str());
    char *temp_id = strtok(buf,":");
    sid = atoi(temp_id);
    temp_id = strtok(NULL,":");
    did = atoi(temp_id);

	path = "/home/yzr/common/data/";
	bind("pertime_", &pertime_);
	bind("isSat_", &isSat_);
	for(int i=0; i<3; i++){
		allflow_td[i] = -1;
	}
	wholetime = 0;
	countTp_gd = 0;

	if(isSat_ == 0) {
		ofstream of;
		char txt[10];
		//初始化吞吐量文件
		sprintf(txt, "%d-%d", sid, did);
		string s1 = txt;
		string s2 = path+"tp_gd"+s1+".txt";
		of.open(s2.c_str(), ios::out);
		of.close();

		//初始化时延文件
		sprintf(txt, "%d", sid);
		s1 = txt;
		s2 = path+"td_gd"+s1+".txt";
		of.open(s2.c_str(), ios::out);
		of.close();
	}
	countTd_gd = 0;
	countTdpkt_gd = 0;
	testpkt = 0;

}
NoQue::~NoQue(){
	delete q_;
}

void NoQue::enque(Packet *pkt) {
	//cout << "包来了" << endl;

	double t = Scheduler::instance().clock();
	hdr_cmn* ch = hdr_cmn::access(pkt);
	countTd_gd += t-ch->timestamp();
	countTdpkt_gd++;
	q_->enque(pkt);

}

Packet *NoQue::deque() {
	
	Packet* p;
	p = q_->deque();

	double t = Scheduler::instance().clock();
	if((t-wholetime) > pertime_){
		if(isSat_ == 1){
			recordPktcnt_allflowpertd();
		}else{
			recordTp_gd();
			recordTd_gd();
		}
		wholetime += pertime_;
		//cout << wholetime << endl;
		testpkt = 0;
	}

	if(p != NULL){
		countTp_gd++;
		

		hdr_cmn* ch = hdr_cmn::access(p);
		hdr_ip* iph = hdr_ip::access(p);

		if((isSat_==1 && ch->next_hop_-70)>=0){
			if(allflow_td[ch->next_hop_-70] == -1){
				ofstream of;
				char txt[2];
				sprintf(txt, "%d",iph->daddr()-70);
				string s = txt;
				string s1 = path+"perFlowTimeDelay"+s+".txt";
				of.open(s1.c_str(), ios::out);
				of.close();
				allflow_td[ch->next_hop_-70] = 0;
				allflow_td_pkt[ch->next_hop_-70] = 0;
			}
			allflow_td[ch->next_hop_-70] += t-ch->timestamp();
			allflow_td_pkt[ch->next_hop_-70]++;
		}


	}
	return (p);
}

void NoQue::recordPktcnt_allflowpertd(){
	double t = Scheduler::instance().clock();
	for(int i=0; i<3; i++){
		if(allflow_td[i] >= 0){
			ofstream of;
			char txt[2];
			sprintf(txt, "%d",i);
			string s = txt;
			string s1 = path+"perFlowTimeDelay"+s+".txt";
			of.open(s1.c_str(), ios::app);
			of << t << " " << allflow_td[i]/allflow_td_pkt[i] << endl;
			of.close();
			allflow_td[i] = 0;
			allflow_td_pkt[i] = 0;
		}
	}
}

void NoQue::recordTp_gd(){
	double t = Scheduler::instance().clock();
	ofstream of;
	char txt[10];
	sprintf(txt, "%d-%d", sid, did);
	string s1 = txt;
	string s2 = path+"tp_gd"+s1+".txt";
	of.open(s2.c_str(), ios::app);
	of << t << " " << countTp_gd << endl;
	countTp_gd = 0;
	of.close();
}

void NoQue::recordTd_gd(){
	double t = Scheduler::instance().clock();
	ofstream of;
	char txt[10];
	sprintf(txt, "%d", sid);
	string s1 = txt;
	string s2 = path+"td_gd"+s1+".txt";
	of.open(s2.c_str(), ios::app);
	if(countTdpkt_gd != 0){
		// of << t << " " << countTd_gd/countTdpkt_gd << endl;
		of << countTd_gd/countTdpkt_gd << endl;
	}else {
		// of << t << " " << 0 << endl;
		of << 0 << endl;
	}
	
	countTd_gd = 0;
	countTdpkt_gd = 0;
	of.close();
}

int NoQue::command(int argc, const char*const* argv)
{
	if (argc==2) {
		if (strcmp(argv[1], "writeover") == 0) {
			return (TCL_OK);
		}

	}

	return (Queue::command(argc, argv));
}
