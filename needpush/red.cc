 /* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 1990-1997 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Computer Systems
 *	Engineering Group at Lawrence Berkeley Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * Here is one set of parameters from one of Sally's simulations
 * (this is from tcpsim, the older simulator):
 * 
 * ed [ q_weight=0.002 thresh=5 linterm=30 maxthresh=15
 *         mean_pktsize=500 dropmech=random-drop queue-size=60
 *         plot-file=none bytes=false doubleq=false dqthresh=50 
 *	   wait=true ]
 * 
 * 1/"linterm" is the max probability of dropping a packet. 
 * There are different options that make the code
 * more messy that it would otherwise be.  For example,
 * "doubleq" and "dqthresh" are for a queue that gives priority to
 *   small (control) packets, 
 * "bytes" indicates whether the queue should be measured in bytes 
 *   or in packets, 
 * "dropmech" indicates whether the drop function should be random-drop 
 *   or drop-tail when/if the queue overflows, and 
 *   the commented-out Holt-Winters method for computing the average queue 
 *   size can be ignored.
 * "wait" indicates whether the gateway should wait between dropping
 *   packets.
 */

#ifndef lint
static const char rcsid[] =
     "@(#) $Header: /cvsroot/nsnam/ns-2/queue/red.cc,v 1.90 2011/10/02 22:32:34 tom_henderson Exp $ (LBL)";
#endif

#include <math.h>
#include <sys/types.h>
#include "config.h"
#include "template.h"
#include "random.h"
#include "flags.h"
#include "delay.h"
#include "red.h"

static class REDClass : public TclClass {
public:
	REDClass() : TclClass("Queue/RED") {}
	TclObject* create(int argc, const char*const* argv) {
		//printf("creating RED Queue. argc = %d\n", argc);
		string s = argv[4];
		//mod to enable RED to take arguments
		// cout<<"参数" << argv[0] << " "<< argv[1] << " "<< argv[2] << " "<< argv[3] << " "<< argv[4] << " "<<endl;
		// cout<<"参数个数" << argc<<endl;
		if (argc==5) 
			//return (new REDQueue(argv[4]));
			return (new REDQueue("Drop", s));
		else 
			return (new REDQueue("Drop", s));
	}
} class_red;

/* Strangely this didn't work. 
 * Seg faulted for child classes.
REDQueue::REDQueue() { 
	REDQueue("Drop");
}
*/

/*
 * modified to enable instantiation with special Trace objects - ratul
 */
// REDQueue::REDQueue(const char * trace) : link_(NULL), de_drop_(NULL), EDTrace(NULL), tchan_(0), idle_(1), idletime_(0.0)
// {
// 	initParams();
	
// 	//	printf("Making trace type %s\n", trace);
// 	if (strlen(trace) >=20) {
// 		printf("trace type too long - allocate more space to traceType in red.h and recompile\n");
// 		exit(0);
// 	}
// 	strcpy(traceType, trace);
// 	bind_bool("bytes_", &edp_.bytes);	    // boolean: use bytes?
// 	bind_bool("queue_in_bytes_", &qib_);	    // boolean: q in bytes?
// 	//	_RENAMED("queue-in-bytes_", "queue_in_bytes_");

// 	bind("thresh_", &edp_.th_min_pkts);		    // minthresh
// 	bind("thresh_queue_", &edp_.th_min);
// 	bind("maxthresh_", &edp_.th_max_pkts);	    // maxthresh
// 	bind("maxthresh_queue_", &edp_.th_max);
// 	bind("mean_pktsize_", &edp_.mean_pktsize);  // avg pkt size
// 	bind("idle_pktsize_", &edp_.idle_pktsize);  // avg pkt size for idles
// 	bind("q_weight_", &edp_.q_w);		    // for EWMA
// 	bind("adaptive_", &edp_.adaptive);          // 1 for adaptive red
// 	bind("cautious_", &edp_.cautious);          // 1 for cautious marking
// 	bind("alpha_", &edp_.alpha); 	  	    // adaptive red param
// 	bind("beta_", &edp_.beta);                  // adaptive red param
// 	bind("interval_", &edp_.interval);	    // adaptive red param
// 	bind("feng_adaptive_",&edp_.feng_adaptive); // adaptive red variant
// 	bind("targetdelay_", &edp_.targetdelay);    // target delay
// 	bind("top_", &edp_.top);		    // maximum for max_p	
// 	bind("bottom_", &edp_.bottom);		    // minimum for max_p	
// 	bind_bool("wait_", &edp_.wait);
// 	bind("linterm_", &edp_.max_p_inv);
// 	bind("mark_p_", &edp_.mark_p);
// 	bind_bool("use_mark_p_", &edp_.use_mark_p);
// 	bind_bool("setbit_", &edp_.setbit);	    // mark instead of drop
// 	bind_bool("gentle_", &edp_.gentle);         // increase the packet
// 						    // drop prob. slowly
// 						    // when ave queue
// 						    // exceeds maxthresh

// 	bind_bool("summarystats_", &summarystats_);
// 	bind_bool("drop_tail_", &drop_tail_);	    // drop last pkt
// 	//	_RENAMED("drop-tail_", "drop_tail_");

// 	bind_bool("drop_front_", &drop_front_);	    // drop first pkt
// 	//	_RENAMED("drop-front_", "drop_front_");
	
// 	bind_bool("drop_rand_", &drop_rand_);	    // drop pkt at random
// 	//	_RENAMED("drop-rand_", "drop_rand_");

// 	bind_bool("ns1_compat_", &ns1_compat_);	    // ns-1 compatibility
// 	//	_RENAMED("ns1-compat_", "ns1_compat_");

// 	bind("ave_", &edv_.v_ave);		    // average queue sie
// 	bind("prob1_", &edv_.v_prob1);		    // dropping probability
// 	bind("curq_", &curq_);			    // current queue size
// 	bind("cur_max_p_", &edv_.cur_max_p);        // current max_p
// 	bind("pertime_", &pertime_);
// 	bind("isSat_", &isSat_);

// 	q_ = new PacketQueue();			    // underlying queue
// 	pq_ = q_;
// 	//reset();
// #ifdef notdef
// 	print_edp();
// 	print_edv();
// #endif

// }

REDQueue::REDQueue(const char * trace, string s) : link_(NULL), de_drop_(NULL), EDTrace(NULL), tchan_(0), idle_(1), idletime_(0.0)
{
	initParams();
	
	//	printf("Making trace type %s\n", trace);
	if (strlen(trace) >=20) {
		printf("trace type too long - allocate more space to traceType in red.h and recompile\n");
		exit(0);
	}
	strcpy(traceType, trace);
	bind_bool("bytes_", &edp_.bytes);	    // boolean: use bytes?
	bind_bool("queue_in_bytes_", &qib_);	    // boolean: q in bytes?
	//	_RENAMED("queue-in-bytes_", "queue_in_bytes_");

	bind("thresh_", &edp_.th_min_pkts);		    // minthresh 按包计算
	bind("thresh_queue_", &edp_.th_min);			//按照byte计算
	bind("maxthresh_", &edp_.th_max_pkts);	    // maxthresh
	bind("maxthresh_queue_", &edp_.th_max);
	bind("mean_pktsize_", &edp_.mean_pktsize);  // avg pkt size
	bind("idle_pktsize_", &edp_.idle_pktsize);  // avg pkt size for idles
	bind("q_weight_", &edp_.q_w);		    // for EWMA
	bind("ared_", &edp_.ared);          // 1 for adaptive red
	bind("cautious_", &edp_.cautious);          // 1 for cautious marking
	bind("alpha_", &edp_.alpha); 	  	    // adaptive red param
	bind("beta_", &edp_.beta);                  // adaptive red param
	bind("interval_", &edp_.interval);	    // adaptive red param
	bind("feng_adaptive_",&edp_.feng_adaptive); // adaptive red variant
	bind("targetdelay_", &edp_.targetdelay);    // target delay
	bind("top_", &edp_.top);		    // maximum for max_p	
	bind("bottom_", &edp_.bottom);		    // minimum for max_p	
	bind_bool("wait_", &edp_.wait);
	bind("linterm_", &edp_.max_p_inv);
	bind("mark_p_", &edp_.mark_p);
	bind_bool("use_mark_p_", &edp_.use_mark_p);
	bind_bool("setbit_", &edp_.setbit);	    // mark instead of drop
	bind_bool("gentle_", &edp_.gentle);         // increase the packet
						    // drop prob. slowly
						    // when ave queue
						    // exceeds maxthresh

	bind_bool("summarystats_", &summarystats_);
	bind_bool("drop_tail_", &drop_tail_);	    // drop last pkt
	//	_RENAMED("drop-tail_", "drop_tail_");

	bind_bool("drop_front_", &drop_front_);	    // drop first pkt
	//	_RENAMED("drop-front_", "drop_front_");
	
	bind_bool("drop_rand_", &drop_rand_);	    // drop pkt at random
	//	_RENAMED("drop-rand_", "drop_rand_");

	bind_bool("ns1_compat_", &ns1_compat_);	    // ns-1 compatibility
	//	_RENAMED("ns1-compat_", "ns1_compat_");

	bind("ave_", &edv_.v_ave);		    // average queue sie
	bind("prob1_", &edv_.v_prob1);		    // dropping probability
	bind("curq_", &curq_);			    // current queue size
	bind("cur_max_p_", &edv_.cur_max_p);        // current max_p
	bind("cared_", &cared);
	bind("cfared_", &cfared);
	bind("p_ared_", &p_ared);
	bind("shape_", &shape);
	bind("pertime_", &pertime_);
	bind("isSat_", &isSat_);

	q_ = new PacketQueue();			    // underlying queue
	pq_ = q_;
	//reset();
#ifdef notdef
	print_edp();
	print_edv();
#endif

	//获取链路的两端
	char buf[10];
	strcpy(buf, s.c_str());
    char *temp_id = strtok(buf,":");
    sid = atoi(temp_id);
    temp_id = strtok(NULL,":");
    did = atoi(temp_id);

	if(isSat_ == 0) {
		ofstream of;
		char txt[10];
		//初始化吞吐量文件
		sprintf(txt, "%d-%d", sid, did);
		string s1 = txt;
		string s2 = path+"tp_gd"+s1+".txt";
		of.open(s2.c_str(), ios::out);
		of.close();

		//初始化丢包率文件
		sprintf(txt, "%d-%d", sid, did);
		s1 = txt;
		s2 = path+"dr_gd"+s1+".txt";
		of.open(s2.c_str(), ios::out);
		of.close();
	}

}

REDQueue::~REDQueue() {

	flosspkt1.close();
	flosspkt2.close();
	fthroughput1.close();
	fthroughput2.close();
	ftimedelay1.close();
	ftimedelay2.close();
	test_itv.close();
}


/*
 * Note: if the link bandwidth changes in the course of the
 * simulation, the bandwidth-dependent RED parameters do not change.
 * This should be fixed, but it would require some extra parameters,
 * and didn't seem worth the trouble...
 */
void REDQueue::initialize_params()
{

/*
 * If q_weight=0, set it to a reasonable value of 1-exp(-1/C)
 * This corresponds to choosing q_weight to be of that value for
 * which the packet time constant -1/ln(1-q_weight) per default RTT 
 * of 100ms is an order of magnitude more than the link capacity, C.
 *
 * If q_weight=-1, then the queue weight is set to be a function of
 * the bandwidth and the link propagation delay.  In particular, 
 * the default RTT is assumed to be three times the link delay and 
 * transmission delay, if this gives a default RTT greater than 100 ms. 
 *
 * If q_weight=-2, set it to a reasonable value of 1-exp(-10/C).
 */
	//printf("ptc=%lf    delay=%lf\n", edp_.ptc, edp_.delay);
	if (edp_.q_w == 0.0) {
		edp_.q_w = 1.0 - exp(-1.0/edp_.ptc);
 	} else if (edp_.q_w == -1.0) {
		double rtt = 3.0*(edp_.delay+1.0/edp_.ptc);
		//printf("delay: %5.4f rtt: %5.4f\n", edp_.delay, rtt);
		if (rtt < 0.1) 
			rtt = 0.1;
		edp_.q_w = 1.0 - exp(-1.0/(10*rtt*edp_.ptc));
	} else if (edp_.q_w == -2.0) {
		edp_.q_w = 1.0 - exp(-10.0/edp_.ptc);
	}
	// printf("ptc: %7.5f bandwidth: %5.3f pktsize: %d\n", edp_.ptc, link_->bandwidth(), edp_.mean_pktsize);
        // printf("th_min_pkts: %7.5f th_max_pkts: %7.5f\n", edp_.th_min_pkts, edp_.th_max);
	if (edp_.th_min_pkts == 0) {
		edp_.th_min_pkts = 5.0;
		// set th_min_pkts to half of targetqueue, if this is greater
		//  than 5 packets.
		double targetqueue = edp_.targetdelay * edp_.ptc;
		if (edp_.th_min_pkts < targetqueue / 2.0 )
			edp_.th_min_pkts = targetqueue / 2.0 ;
        }
	if (edp_.th_max_pkts == 0) 
		edp_.th_max_pkts = 3.0 * edp_.th_min_pkts;
        //printf("th_min_pkts: %7.5f th_max_pkts: %7.5f\n", edp_.th_min_pkts, edp_.th_max);
	//printf("q_w: %7.5f\n", edp_.q_w);
	if (edp_.bottom == 0) {
		edp_.bottom = 0.01;
		// Set bottom to at most 1/W, for W the delay-bandwidth 
		//   product in packets for a connection with this bandwidth,
		//   1000-byte packets, and 100 ms RTTs.
		// So W = 0.1 * link_->bandwidth() / 8000 
		double bottom1 = 80000.0/link_->bandwidth();
		if (bottom1 < edp_.bottom) 
			edp_.bottom = bottom1;
		//printf("bottom: %9.7f\n", edp_.bottom);
	}


}

//初始化参数值，简单归零
void REDQueue::initParams() 
{

	edp_.mean_pktsize = 0;
	edp_.idle_pktsize = 0;
	edp_.bytes = 0;
	edp_.wait = 0;
	edp_.setbit = 0;
	edp_.gentle = 0;
	edp_.th_min = 0.0;
	edp_.th_min_pkts = 0.0;
	edp_.th_max = 0.0;
	edp_.th_max_pkts = 0.0;
	edp_.max_p_inv = 0.0;
	edp_.q_w = 0.0;
	edp_.ared = 0;
	edp_.cautious = 0;
	edp_.alpha = 0.0;
	edp_.beta = 0.0;
	edp_.interval = 0.0;
	edp_.targetdelay = 0.0;
	edp_.top = 0.0;
	edp_.bottom = 0.0;
	edp_.feng_adaptive = 0;
	edp_.ptc = 0.0;
	edp_.delay = 0.0;
	
	edv_.v_ave = 0.0;
	edv_.v_oldave = 0.0;
	edv_.v_prob1 = 0.0;
	edv_.v_slope = 0.0;
	edv_.v_prob = 0.0;
	edv_.v_a = 0.0;
	edv_.v_b = 0.0;
	edv_.v_c = 0.0;
	edv_.v_d = 0.0;
	edv_.count = 0;
	edv_.count_bytes = 0;
	edv_.old = 0;
	edv_.cur_max_p = 1.0;
	edv_.lastset = 0;

E1=968.70170 ;
D1=138.60679 ;
	pktcnt_pers = 0;
	pktcnt_pers_lp = 0;
	pktcnt_pers_tp = 0;
	pktcnt_pers_td = 0;
	pktcnt_pers_maxp = 0;

per_ave = 0;
count_ave = 0;
per_pktlen = 0;
droppkt = 0;
droppkt_10 = 0;

	drop_cnt_pers = 0;
	tp_cnt_pers = 0;
	timeDelay_pers = 0;
	static double temp[6] ={0.00000 , 3.11765 , 3.28252 , 3.71305 , 4.06667 , 0.00000 };

	scale = temp;
	ek[0] = 0;
	ek[1] = 0;
	ek[2] = 0;
	for(int i = 0; i<9; i++){
		xk[i] = 0;
		yk[i] = 0;
		wk[i] = 0;
		wk1[i] = 0;
	}
	tin = -1;
	// test_itv.open("/home/yzr/common/papersims/yzr_interval.txt",ios::out);
	// fperpkt1.open("/home/yzr/common/papersims/yzr_perpkt.txt",ios::out);
	// fperpkt2.open("/home/yzr/common/papersims/yzr_perpkt_d.txt",ios::out);
	// fperpkt1 << "\"Packet Arrival" <<endl;
	// flosspkt1.open("/home/yzr/common/papersims/yzr_queue_dr.txt",ios::out);
	// flosspkt1 << "\"Packet loss rate" <<endl;
	// flosspkt2.open("/home/yzr/common/papersims/yzr_queue_dr_d.txt",ios::out);
	// fthroughput1.open("/home/yzr/common/papersims/yzr_queue_tp.txt",ios::out);
	// fthroughput1 << "\"Through put" <<endl;
	// fthroughput2.open("/home/yzr/common/papersims/yzr_queue_tp_d.txt",ios::out);
	// ftimedelay1.open("/home/yzr/common/papersims/yzr_queue_td.txt",ios::out);
	// ftimedelay1 << "\"Time delay" <<endl;
	// ftimedelay2.open("/home/yzr/common/papersims/yzr_queue_td_d.txt",ios::out);
	// faveplen.open("/home/yzr/common/papersims/yzr_aveqlen.txt",ios::out);
	// faveplen << "\"Avelen" <<endl;
	// fdrop_10.open("/home/yzr/common/papersims/yzr_drop_10.txt",ios::out);
	fSatTp.open("/home/yzr/common/yzr_drop_10.txt",ios::out);

	//读取等级
	levelnum = 0;
	ifstream ifs;
	ifs.open("/home/yzr/common/papersims/level.txt",ios::in);
	string str;
	int num = 0;
	int tmp;
	while(getline(ifs, str)){
		tmp = atoi(str.c_str());
		mylevel[num] = tmp;
		num++;
	}

	for(int i=0; i<82; i++){
		allnode_tp[i] = -1;
	}
	for(int i=0; i<3; i++){
		allflow_dp[i] = -1;
	}
	wholetime = 0;
	path = "/home/yzr/common/data/";
	countTp_gd = 0;
	countDr_gd = 0;
	testpkt = 0;
}

//重新设置参数值
void REDQueue::reset()
{
	
        //printf("3: th_min_pkts: %5.2f\n", edp_.th_min_pkts); 
	/*
	 * Compute the "packet time constant" if we know the
	 * link bandwidth.  The ptc is the max number of (avg sized)
	 * pkts per second which can be placed on the link.
	 * The link bw is given in bits/sec, so scale mean psize
	 * accordingly.
	 */
        if (link_) {
		edp_.ptc = link_->bandwidth() / (8.0 * edp_.mean_pktsize);
		initialize_params();
	}
	if (edp_.th_max_pkts == 0) 
		edp_.th_max_pkts = 3.0 * edp_.th_min_pkts;
	/*
	 * If queue is measured in bytes, scale min/max thresh
	 * by the size of an average packet (which is specified by user).
	 */
        if (qib_) {
		//printf("1: th_min in pkts: %5.2f mean_pktsize: %d \n", edp_.th_min_pkts, edp_.mean_pktsize); 
                edp_.th_min = edp_.th_min_pkts * edp_.mean_pktsize;  
                edp_.th_max = (double)edp_.th_max_pkts * edp_.mean_pktsize;
        		// edp_.th_min = 4000;  
          //       edp_.th_max = 10000;
		//printf("2: th_min in bytes (if qib): %5.2f mean_pktsize: %d \n", edp_.th_min, edp_.mean_pktsize); 
        } else {
		edp_.th_min = edp_.th_min_pkts;
		edp_.th_max = edp_.th_max_pkts;

	}
	 
	edv_.v_ave = 0.0;
	edv_.v_slope = 0.0;
	edv_.count = 0;
	edv_.count_bytes = 0;
	edv_.old = 0;
	double th_diff = (edp_.th_max - edp_.th_min);
	if (th_diff == 0) { 
		//XXX this last check was added by a person who knows
		//nothing of this code just to stop FP div by zero.
		//Values for thresholds were equal at time 0.  If you
		//know what should be here, please cleanup and remove
		//this comment.
		th_diff = 1.0; 
	}

	edv_.v_a = 1.0 / th_diff;
	edv_.cur_max_p = 1.0 / edp_.max_p_inv;
	edv_.v_b = - edp_.th_min / th_diff;
	edv_.lastset = 0.0;

	if (edp_.gentle) {
		edv_.v_c = ( 1.0 - edv_.cur_max_p ) / edp_.th_max;
		edv_.v_d = 2.0 * edv_.cur_max_p - 1.0;
	}
	idle_ = 1;
	if (&Scheduler::instance() != NULL)
		idletime_ = Scheduler::instance().clock();
	else
		idletime_ = 0.0; /* sched not instantiated yet */
	if (debug_) 
		printf("Doing a queue reset\n");
	Queue::reset();
	if (debug_) 
		printf("Done queue reset\n");


}

/*
 *  Updating max_p, following code from Feng et al. 
 *  This is only called for Adaptive RED.
 *  From "A Self-Configuring RED Gateway", from Feng et al.
 *  They recommend alpha = 3, and beta = 2.
 */
void REDQueue::updateMaxPFeng(double new_ave)
{
	if ( edp_.th_min < new_ave && new_ave < edp_.th_max) {
		edv_.status = edv_.Between;
	}
	if (new_ave < edp_.th_min && edv_.status != edv_.Below) {
		edv_.status = edv_.Below;
		edv_.cur_max_p = edv_.cur_max_p / edp_.alpha;
		//double max = edv_.cur_max_p; double param = edp_.alpha;
		//printf("max: %5.2f alpha: %5.2f\n", max, param);
	}
	if (new_ave > edp_.th_max && edv_.status != edv_.Above) {
		edv_.status = edv_.Above;
		edv_.cur_max_p = edv_.cur_max_p * edp_.beta;
		//double max = edv_.cur_max_p; double param = edp_.alpha;
		//printf("max: %5.2f beta: %5.2f\n", max, param);
	}
}

/*
 *  Updating max_p to keep the average queue size within the target range.
 *  This is only called for Adaptive RED.
 */
void REDQueue::updateMaxP(double new_ave, double now)
{
	//usleep(100000);
	//cout << "ared" << endl;
	double part = 0.4*(edp_.th_max - edp_.th_min);
	// printf("th_max:%lf   \n", edp_.th_max);
	// printf("th_min:%lf   \n", edp_.th_min);
	// printf("ave:%lf   \n", new_ave);

	// AIMD rule to keep target Q~1/2(th_min+th_max)
	if ( new_ave < edp_.th_min + part && edv_.cur_max_p > edp_.bottom) {
		// we increase the average queue size, so decrease max_p
		edv_.cur_max_p = edv_.cur_max_p * edp_.beta;
		edv_.lastset = now;
	} else if (new_ave > edp_.th_max - part && edp_.top > edv_.cur_max_p ) {
		// we decrease the average queue size, so increase max_p
		double alpha = edp_.alpha;
                        if ( alpha > 0.25*edv_.cur_max_p )
			alpha = 0.25*edv_.cur_max_p;
		edv_.cur_max_p = edv_.cur_max_p + alpha;
		edv_.lastset = now;
	} 
	edv_.lastset = now;
	//printf("now=%lf     edv_.lastset=%lf\n",now, edv_.lastset);
}


double REDQueue::getLevel() {
	//double t = Scheduler::instance().clock();
	double level = 0;
	//printf("pktcnt_pers_level=%d\n",pktcnt_pers_level);
	//printf("t=%lf\n",t);
	if(pktcnt_pers_level < E1-4*D1) {
		level = 1;
	}else if (pktcnt_pers_level >= E1-4*D1 && pktcnt_pers_level < E1-2*D1) {
		level = 2;
	}else if (pktcnt_pers_level >= E1-2*D1 && pktcnt_pers_level < E1) {
		level = 3;
	}else if (pktcnt_pers_level >= E1 && pktcnt_pers_level <E1+2*D1) {
		level = 4;
	}else if (pktcnt_pers_level >= E1+2*D1 && pktcnt_pers_level < E1+4*D1) {
		level = 5;
	}else if (pktcnt_pers_level >= E1+4*D1) {
		level = 6;
	}

	pktcnt_pers_level = 0;
	//printf("level=%lf\n",level);


	return level;
}


void REDQueue::updateMaxP_CF(double new_ave, double now)
{
	double alpha = 0.01<edv_.cur_max_p/4?0.01:edv_.cur_max_p/4;
	double beta = 0.9;
	double thresh_ = edp_.th_min;
	double maxthresh_ = edp_.th_max;
	double target_low = thresh_ + 0.4*(maxthresh_ - thresh_);
	double target_up = thresh_ + 0.6*(maxthresh_ - thresh_);

	if(new_ave < target_low && edv_.cur_max_p >= 0.1){
		edv_.cur_max_p = edv_.cur_max_p * beta;
	}else if(new_ave > target_up && edv_.cur_max_p <= 0.5){
		edv_.cur_max_p = edv_.cur_max_p + alpha;
	}

	edv_.lastset = now;
	//printf("now=%lf     edv_.lastset=%lf\n",now, edv_.lastset);
}

void REDQueue::updateMaxP_C(double new_ave, double now)
{
	double alpha;
	double beta = 0.9;
	double thresh_ = edp_.th_min;
	double maxthresh_ = edp_.th_max;
	double target_low = thresh_ + 0.4*(maxthresh_ - thresh_);
	double target_up = thresh_ + 0.6*(maxthresh_ - thresh_);

	if(new_ave < target_low && edv_.cur_max_p >= 0.01){
		if(new_ave > edv_.v_oldave){
			edv_.cur_max_p = edv_.cur_max_p*beta;
		}else if(new_ave < edv_.v_oldave){
			beta = 1-(0.17*(target_low-new_ave)/(target_low-thresh_));
			edv_.cur_max_p = edv_.cur_max_p*beta;
		}
	}else if(new_ave > target_up && edv_.cur_max_p <= 0.5){
		if(new_ave > edv_.v_oldave){
			alpha = 0.25*edv_.cur_max_p*(new_ave-target_up)/target_up;
			edv_.cur_max_p = edv_.cur_max_p+alpha;
		}else if(new_ave < edv_.v_oldave){
			alpha = 0.25*edv_.cur_max_p<0.01?0.25*edv_.cur_max_p:0.01;
			edv_.cur_max_p = edv_.cur_max_p+alpha;
		}
	}

	edv_.lastset = now;
	//printf("now=%lf     edv_.lastset=%lf\n",now, edv_.lastset);
}

void REDQueue::updateMaxP_N(double new_ave, double now)
{
	double thresh_ = edp_.th_min;
	double maxthresh_ = edp_.th_max;
	double level = getLevel();
	double temp_pmax = 0.1;
	double H = (3-shape)/2;


	if(thresh_ <= new_ave && new_ave <= 1.25*thresh_){
		edv_.cur_max_p =  temp_pmax/(2-2/3*pow((level-1)/2,H/2));
	}else if (1.25*thresh_ < new_ave && new_ave < 0.75*maxthresh_){
		edv_.cur_max_p = temp_pmax;
	}else if(0.75*maxthresh_ <= new_ave && new_ave<=maxthresh_){
		edv_.cur_max_p = temp_pmax * (1 + 1/2 * pow((level-1)/3, H));
	}

	edv_.lastset = now;
	//printf("now=%lf     edv_.lastset=%lf\n",now, edv_.lastset);
}



/*
 * Compute the average queue size.
 * Nqueued can be bytes or packets.
 */
/* estimator参数列表: 
 * nqueued：瞬时队列长度
 * m：未知？
 * ave：平均队列长度
 * q_w：权值
 */
double REDQueue::estimator(int nqueued, int m, double ave, double q_w)
{

	double new_ave;

	new_ave = ave;
	while (--m >= 1) {
		new_ave *= 1.0 - q_w;
	}
	new_ave *= 1.0 - q_w;
	new_ave += q_w * nqueued;
	
	double now = Scheduler::instance().clock();
	if (edp_.ared == 1) {
		if (edp_.feng_adaptive == 1)
			updateMaxPFeng(new_ave);
		else if (now > edv_.lastset + edp_.interval)
			updateMaxP(new_ave, now);
	}
	if(cared == 1){
		if(now > edv_.lastset + pertime_){
			updateMaxP_C(new_ave, now);
		}
	}
	if(cfared == 1){
		if(now > edv_.lastset + pertime_){
			updateMaxP_CF(new_ave, now);
		}
	}
	if(p_ared == 1){
		if(now > edv_.lastset + pertime_){
			updateMaxP_N(new_ave, now);
		}
	}

	return new_ave;
}

/*
 * Return the next packet in the queue for transmission.
 */
Packet* REDQueue::deque()
{
	
	double t = Scheduler::instance().clock();
	if((t-wholetime) > pertime_){
		if(isSat_ == 1){
			recordPktcnt_allpertp();
			recordPktcnt_allflowperdp(wholetime);
		}else{
			recordTp_gd();
			recordDr_gd();
		}
		
		wholetime += pertime_;
		cout << wholetime << endl;
		testpkt = 0;
	}
	Packet *p;
	if (summarystats_ && &Scheduler::instance() != NULL) {
		//计算真实平均队列长度：约等于总长度比当前仿真总时间
		Queue::updateStats(qib_?q_->byteLength():q_->length());
	}
	p = q_->deque();
	
	if (p != 0) {
		countTp_gd++;
		idle_ = 0;
		hdr_cmn* ch = hdr_cmn::access(p);
		hdr_ip* iph = hdr_ip::access(p);

		if(isSat_ == 1){
			if(allnode_tp[ch->next_hop_] == -1){
				ofstream of;
				char txt[10];
				sprintf(txt, "(%d-%d)", sid, ch->next_hop_);
				string s = txt;
				s = path+"perThroughput"+s+".txt";
				of.open(s.c_str(), ios::out);
				of.close();
			}
			if((iph->daddr()-70)>=0 && sid == 1 && allflow_dp[iph->daddr()-70] == -1){
				ofstream of;
				char txt[2];
				sprintf(txt, "%d",iph->daddr()-70);
				string s = txt;
				string s1 = path+"perFlowDropRate"+s+".txt";
				of.open(s1.c_str(), ios::out);
				of.close();
				allflow_dp[iph->daddr()-70] = 0;
			}

			allnode_tp[ch->next_hop_]++;
		}

		
		
			pktcnt_pers_td++;
			pktcnt_pers_tp++;
			tp_cnt_pers++;
			// if(t-ch->timestamp() < 0.025){
			// 	printf("sport=%ld dport=%ld time=%lf\n",iph->saddr(), iph->daddr(), (t-ch->timestamp())); 
			// }
		
	} else {
		idle_ = 1;
		// deque() may invoked by Queue::reset at init
		// time (before the scheduler is instantiated).
		// deal with this case
		if (&Scheduler::instance() != NULL)
			idletime_ = Scheduler::instance().clock();
		else
			idletime_ = 0.0;
	}

	return (p);
}

/*
 * Calculate the drop probability.
 */
double
REDQueue::calculate_p_new(double v_ave, double th_max, int gentle, double v_a, 
	double v_b, double v_c, double v_d, double max_p)
{
	double p;
	if (gentle && v_ave >= th_max) {
		// p ranges from max_p to 1 as the average queue
		// size ranges from th_max to twice th_max 
		p = v_c * v_ave + v_d;
        } else if (!gentle && v_ave >= th_max) { 
                // OLD: p continues to range linearly above max_p as
                // the average queue size ranges above th_max.
                // NEW: p is set to 1.0 
                p = 1.0;
        } else {
                // p ranges from 0 to max_p as the average queue
                // size ranges from th_min to th_max 
                p = v_a * v_ave + v_b;
                // p = (v_ave - th_min) / (th_max - th_min)
                p *= max_p; 
        }
	if (p > 1.0)
		p = 1.0;
	return p;
}

/*
 * Calculate the drop probability.
 * This is being kept for backwards compatibility.
 */
double
REDQueue::calculate_p(double v_ave, double th_max, int gentle, double v_a, 
	double v_b, double v_c, double v_d, double max_p_inv)
{
	double p = calculate_p_new(v_ave, th_max, gentle, v_a,
		v_b, v_c, v_d, 1.0 / max_p_inv);
	return p;
}

/*
 * Make uniform instead of geometric interdrop periods.
 */
double
REDQueue::modify_p(double p, int count, int count_bytes, int bytes, 
   int mean_pktsize, int wait, int size)
{
	double count1 = (double) count;
	if (bytes)
		count1 = (double) (count_bytes/mean_pktsize);
	if (wait) {
		if (count1 * p < 1.0)
			p = 0.0;
		else if (count1 * p < 2.0)
			p /= (2.0 - count1 * p);
		else
			p = 1.0;
	} else {
		if (count1 * p < 1.0)
			p /= (1.0 - count1 * p);
		else
			p = 1.0;
	}
	if (bytes && p < 1.0) {
		p = (p * size) / mean_pktsize;
		//p = p * (size / mean_pktsize);

	}
	if (p > 1.0)
		p = 1.0;
 	return p;
}

double
REDQueue::modify_p_CF()
{
	double minth = edp_.th_min;
	double maxth = edp_.th_max;
	double p;

	if(edv_.v_ave < minth){
		p = 0;
	}else if(minth<edv_.v_ave && edv_.v_ave< maxth){
		p = edv_.cur_max_p * 3 * pow(edv_.v_ave-minth,2)/pow(maxth-minth,2);
		p -= 2 * pow(edv_.v_ave-minth,3)/pow(maxth-minth,3);
	}else if(edv_.v_ave >= maxth){
		p = 1;
	}
 	return p;
}

double
REDQueue::modify_p_N()
{
	double minth = edp_.th_min;
	double maxth = edp_.th_max;
	double p;
	double H = (3-shape)/2;

	if(edv_.v_ave < minth){
		p = 0;
	}else if(minth<=edv_.v_ave && edv_.v_ave<= maxth){
		p = edv_.cur_max_p * pow((edv_.v_ave-minth)/(maxth-minth),2*H);
		
	}else if(edv_.v_ave > maxth){
		p = 1;
	}
 	return p;
}
/*
 * 
 */

/*
 * should the packet be dropped/marked due to a probabilistic drop?
 */
//0不丢包  1丢包
int
REDQueue::drop_early(Packet* pkt)
{
	hdr_cmn* ch = hdr_cmn::access(pkt);

	edv_.v_prob1 = calculate_p_new(edv_.v_ave, edp_.th_max, edp_.gentle, 
  	  edv_.v_a, edv_.v_b, edv_.v_c, edv_.v_d, edv_.cur_max_p);
	if(cfared == 1){
		edv_.v_prob = modify_p_CF();
	}else if (p_ared == 1){
		edv_.v_prob = modify_p_N();
	}else{
		edv_.v_prob = modify_p(edv_.v_prob1, edv_.count, edv_.count_bytes,
	  		edp_.bytes, edp_.mean_pktsize, edp_.wait, ch->size());
	}


	// drop probability is computed, pick random number and act
	if (edp_.cautious == 1) {
		 // Don't drop/mark if the instantaneous queue is much
		 //  below the average.
		 // For experimental purposes only.
		int qsize = qib_?q_->byteLength():q_->length();
		// pkts: the number of packets arriving in 50 ms
		double pkts = edp_.ptc * 0.05;
		double fraction = pow( (1-edp_.q_w), pkts);
		// double fraction = 0.9;
		if ((double) qsize < fraction * edv_.v_ave) {
			// queue could have been empty for 0.05 seconds
			// printf("fraction: %5.2f\n", fraction);
			return (0);
		}
	}
	double u = Random::uniform();
	if (edp_.cautious == 2) {
                // Decrease the drop probability if the instantaneous
		//   queue is much below the average.
		// For experimental purposes only.
		int qsize = qib_?q_->byteLength():q_->length();
		// pkts: the number of packets arriving in 50 ms
		double pkts = edp_.ptc * 0.05;
		double fraction = pow( (1-edp_.q_w), pkts);
		// double fraction = 0.9;
		double ratio = qsize / (fraction * edv_.v_ave);
		if (ratio < 1.0) {
			// printf("ratio: %5.2f\n", ratio);
			u *= 1.0 / ratio;
		}
	}

	if (u <= edv_.v_prob) {
		// DROP or MARK
		edv_.count = 0;
		edv_.count_bytes = 0;
		hdr_flags* hf = hdr_flags::access(pickPacketForECN(pkt));
		if (edp_.setbit && hf->ect() && 
                     (!edp_.use_mark_p || edv_.v_prob1 < edp_.mark_p)) { 
			hf->ce() = 1; 	// mark Congestion Experienced bit
			// Tell the queue monitor here - call emark(pkt)
			return (0);	// no drop
		} else {
			return (1);	// drop
		}
	}
	return (0);			// no DROP/mark
}

/*
 * Pick packet for early congestion notification (ECN). This packet is then
 * marked or dropped. Having a separate function do this is convenient for
 * supporting derived classes that use the standard RED algorithm to compute
 * average queue size but use a different algorithm for choosing the packet for 
 * ECN notification.
 */
Packet*
REDQueue::pickPacketForECN(Packet* pkt)
{
	return pkt; /* pick the packet that just arrived */
}

/*
 * Pick packet to drop. Having a separate function do this is convenient for
 * supporting derived classes that use the standard RED algorithm to compute
 * average queue size but use a different algorithm for choosing the victim.
 */
Packet*
REDQueue::pickPacketToDrop() 
{
	int victim;

	if (drop_front_)
		victim = min(1, q_->length()-1);
	else if (drop_rand_)
		victim = Random::integer(q_->length());
	else			/* default is drop_tail_ */
		victim = q_->length() - 1;

	return(q_->lookup(victim)); 
}

/*
 * Receive a new packet arriving at the queue.
 * The average queue size is computed.  If the average size
 * exceeds the threshold, then the dropping probability is computed,
 * and the newly-arriving packet is dropped with that probability.
 * The packet is also dropped if the maximum queue size is exceeded.
 *
 * "Forced" drops mean a packet arrived when the underlying queue was
 * full, or when the average queue size exceeded some threshold and no
 * randomization was used in selecting the packet to be dropped.
 * "Unforced" means a RED random drop.
 *
 * For forced drops, either the arriving packet is dropped or one in the
 * queue is dropped, depending on the setting of drop_tail_.
 * For unforced drops, the arriving packet is always the victim.
 */

#define	DTYPE_NONE	0	/* ok, no drop */
#define	DTYPE_FORCED 	1	/* a "forced" drop */
#define	DTYPE_UNFORCED	2	/* an "unforced" (random) drop */

void REDQueue::enque(Packet* pkt)
{
	hdr_cmn* mych;
	// double t = Scheduler::instance().clock();
	mych = hdr_cmn::access(pkt);
	hdr_ip* iph = hdr_ip::access(pkt);
	// if(mych->ptype() == 0 )
	//printf("nid=%d np=%d \n",mych->next_hop_, mych->myprev_hop_);
	//printf("sport=%ld dport=%ld\n",iph->saddr(), iph->daddr()); 
	pktcnt_pers_lp++;
	pktcnt_pers++;
	pktcnt_pers_level++;
	pktcnt_pers_maxp++;
	testpkt++;

	/*
	 * if we were idle, we pretend that m packets arrived during
	 * the idle period.  m is set to be the ptc times the amount
	 * of time we've been idle for
	 */

	/*  print_edp(); */
	int m = 0;
	if (idle_) {
		// A packet that arrives to an idle queue will never
		//  be dropped.
		double now = Scheduler::instance().clock();
		/* To account for the period when the queue was empty. */
		idle_ = 0;
		// Use idle_pktsize instead of mean_pktsize, for
		//  a faster response to idle times.
		if (edp_.cautious == 3) {
			double ptc = edp_.ptc * 
			edp_.mean_pktsize / edp_.idle_pktsize;
			m = int(ptc * (now - idletime_));
		} else
        	m = int(edp_.ptc * (now - idletime_));
	}

	/*
	 * Run the estimator with either 1 new packet arrival, or with
	 * the scaled version above [scaled by m due to idle time]
	 */
	/* 计算平均队列长度
	 *  qib_ 默认是true,
	*/

	edv_.v_oldave = edv_.v_ave;
	edv_.v_ave = estimator(qib_ ? q_->byteLength() : q_->length(), m + 1, edv_.v_ave, edp_.q_w);
	per_ave += edv_.v_ave;
	//cout << per_ave << endl;

	count_ave++;
	//printf("v_ave: %6.4f (%13.12f) q: %d)\n", 
	//	double(edv_.v_ave), double(edv_.v_ave), q_->length());
	if (summarystats_) {
		/* compute true average queue size for summary stats */
		Queue::updateStats(qib_?q_->byteLength():q_->length());
	}

	/*
	 * count and count_bytes keeps a tally of arriving traffic
	 * that has not been dropped (i.e. how long, in terms of traffic,
	 * it has been since the last early drop)
	 */

	hdr_cmn* ch = hdr_cmn::access(pkt);
	++edv_.count;
	edv_.count_bytes += ch->size();
	/*
	 * DROP LOGIC:
	 *	q = current q size, ~q = averaged q size
	 *	1> if ~q > maxthresh, this is a FORCED drop
	 *	2> if minthresh < ~q < maxthresh, this may be an UNFORCED drop
	 *	3> if (q+1) > hard q limit, this is a FORCED drop
	 */

	register double qavg = edv_.v_ave;
	int droptype = DTYPE_NONE;
	int qlen = qib_ ? q_->byteLength() : q_->length();
	int qlim = qib_ ? (qlim_ * edp_.mean_pktsize) : qlim_;

	curq_ = qlen;	// helps to trace queue during arrival, if enabled
	//cout << edp_.th_min <<  "   " << qlen <<  "   " <<  edp_.th_max << endl;
	if (qavg >= edp_.th_min && qlen > 1) {
		if (!edp_.use_mark_p && 
			((!edp_.gentle && qavg >= edp_.th_max) ||
			(edp_.gentle && qavg >= 2 * edp_.th_max))) {
			droptype = DTYPE_FORCED;
		} else if (edv_.old == 0) {
			/* 
			 * The average queue size has just crossed the
			 * threshold from below to above "minthresh", or
			 * from above "minthresh" with an empty queue to
			 * above "minthresh" with a nonempty queue.
			 */
			edv_.count = 1;
			edv_.count_bytes = ch->size();
			edv_.old = 1;
		} else if (drop_early(pkt)) {
			droptype = DTYPE_UNFORCED;
		}
	} else {
		/* No packets are being dropped.  */
		edv_.v_prob = 0.0;
		edv_.old = 0;		
	}
	if (qlen >= qlim) {
		// see if we've exceeded the queue size
		droptype = DTYPE_FORCED;
	}

	if (droptype == DTYPE_UNFORCED) {
		/* pick packet for ECN, which is dropping in this case */
		Packet *pkt_to_drop = pickPacketForECN(pkt);

		/* 
		 * If the packet picked is different that the one that just arrived,
		 * add it to the queue and remove the chosen packet.
		 */
		if (pkt_to_drop != pkt) {
			q_->enque(pkt);
			q_->remove(pkt_to_drop);
			pkt = pkt_to_drop; /* XXX okay because pkt is not needed anymore */
		}

		// deliver to special "edrop" target, if defined
		if (de_drop_ != NULL) {
	
		//trace first if asked 
		// if no snoop object (de_drop_) is defined, 
		// this packet will not be traced as a special case.
			if (EDTrace != NULL) 
				((Trace *)EDTrace)->recvOnly(pkt);

			reportDrop(pkt);
			de_drop_->recv(pkt);
		}else {
			mych = hdr_cmn::access(pkt);
			iph = hdr_ip::access(pkt);
			if(isSat_ == 1){
				if((iph->daddr()-70)>=0 && sid == 1){
					allflow_dp[iph->daddr()-70]++;
					//cout << "概率丢包了  " << ch->next_hop_ << endl;
				}
			}

			if(mych->ptype() != 5){
				drop_cnt_pers++;
			}
			reportDrop(pkt);
			drop(pkt);
			droppkt++;
			droppkt_10++;
			countDr_gd++;
			

			
			// double t = Scheduler::instance().clock();
			 
			// hdr_ip* iph = hdr_ip::access(pkt);
			// printf("sd=%d dd=%d time=%lf  ptype=%d\n",iph->saddr(), iph->daddr(), (t-ch->timestamp()),  ch->ptype());
			//不统计ack包丢包率

		}
	} else {

		/* forced drop, or not a drop: first enqueue pkt */
		q_->enque(pkt);

		/* drop a packet if we were told to */
		if (droptype == DTYPE_FORCED) {
			
			/* drop random victim or last one */
			pkt = pickPacketToDrop();
			mych = hdr_cmn::access(pkt);
			iph = hdr_ip::access(pkt);
			
			if(isSat_ == 1){
				if((iph->daddr()-70)>=0 && sid == 1){
					//cout << "强制丢包了  " << iph->daddr() << endl;
					allflow_dp[iph->daddr()-70]++;
				}
			}


			if(mych->ptype() != 5){
				drop_cnt_pers++;
			}

			q_->remove(pkt);
			reportDrop(pkt);
			drop(pkt);
			droppkt++;
			droppkt_10++;
			countDr_gd++;
			
			

			
			if (!ns1_compat_) {
				// bug-fix from Philip Liu, <phill@ece.ubc.ca>
				edv_.count = 0;
				edv_.count_bytes = 0;
			}
		}
	}

	return;
}

int REDQueue::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if (argc == 2) {
		if (strcmp(argv[1], "reset") == 0) {
			reset();
			return (TCL_OK);
		}
		if (strcmp(argv[1], "test") == 0) {
			printf("这里是简单测试\n");
			return (TCL_OK);
		}
		if (strcmp(argv[1], "early-drop-target") == 0) {
			if (de_drop_ != NULL)
				tcl.resultf("%s", de_drop_->name());
			return (TCL_OK);
		}
		if (strcmp(argv[1], "edrop-trace") == 0) {
			if (EDTrace != NULL) {
				tcl.resultf("%s", EDTrace->name());
				if (debug_) 
					printf("edrop trace exists according to RED\n");
			}
			else {
				if (debug_)
					printf("edrop trace doesn't exist according to RED\n");
				tcl.resultf("0");
			}
			return (TCL_OK);
		}
		if (strcmp(argv[1], "trace-type") == 0) {
			tcl.resultf("%s", traceType);
			return (TCL_OK);
		}
		if (strcmp(argv[1], "printstats") == 0) {
			print_summarystats();
			return (TCL_OK);
		}
 		if (strcmp(argv[1], "alldrop") == 0) {
 			cout << "丢包数："<< droppkt << endl;
 			return (TCL_OK);
 		}
 		
	} 
	else if (argc == 3) {
		// attach a file for variable tracing
		if (strcmp(argv[1], "attach") == 0) {
			int mode;
			const char* id = argv[2];
			tchan_ = Tcl_GetChannel(tcl.interp(), (char*)id, &mode);
			if (tchan_ == 0) {
				tcl.resultf("RED: trace: can't attach %s for writing", id);
				return (TCL_ERROR);
			}
			return (TCL_OK);
		}
		// tell RED about link stats
		if (strcmp(argv[1], "link") == 0) {
			LinkDelay* del = (LinkDelay*)TclObject::lookup(argv[2]);
			if (del == 0) {
				tcl.resultf("RED: no LinkDelay object %s",
					argv[2]);
				return(TCL_ERROR);
			}
			// set ptc now
			link_ = del;
			edp_.ptc = link_->bandwidth() /
				(8.0 * edp_.mean_pktsize);
			edp_.delay = link_->delay();
			if (
			  (edp_.q_w <= 0.0 || edp_.th_min_pkts == 0 ||
					edp_.th_max_pkts == 0))
                        	initialize_params();
			return (TCL_OK);
		}
		if (strcmp(argv[1], "early-drop-target") == 0) {
			NsObject* p = (NsObject*)TclObject::lookup(argv[2]);
			if (p == 0) {
				tcl.resultf("no object %s", argv[2]);
				return (TCL_ERROR);
			}
			de_drop_ = p;
			return (TCL_OK);
		}
		if (strcmp(argv[1], "edrop-trace") == 0) {
			if (debug_) 
				printf("Ok, Here\n");
			NsObject * t  = (NsObject *)TclObject::lookup(argv[2]);
			if (debug_)  
				printf("Ok, Here too\n");
			if (t == 0) {
				tcl.resultf("no object %s", argv[2]);
				return (TCL_ERROR);
			}
			EDTrace = t;
			if (debug_)  
				printf("Ok, Here too too too %d\n", ((Trace *)EDTrace)->type_);
			return (TCL_OK);
		}
		if (!strcmp(argv[1], "packetqueue-attach")) {
			delete q_;
			if (!(q_ = (PacketQueue*) TclObject::lookup(argv[2])))
				return (TCL_ERROR);
			else {
				pq_ = q_;
				return (TCL_OK);
			}
		}
	}
	return (Queue::command(argc, argv));
}

/*
 * Routine called by TracedVar facility when variables change values.
 * Currently used to trace values of avg queue size, drop probability,
 * and the instantaneous queue size seen by arriving packets.
 * Note that the tracing of each var must be enabled in tcl to work.
 */

void
REDQueue::trace(TracedVar* v)
{
	char wrk[500];
	const char *p;

	if (((p = strstr(v->name(), "ave")) == NULL) &&
	    ((p = strstr(v->name(), "prob")) == NULL) &&
	    ((p = strstr(v->name(), "curq")) == NULL) &&
	    ((p = strstr(v->name(), "cur_max_p"))==NULL) ) {
		fprintf(stderr, "RED:unknown trace var %s\n",
			v->name());
		return;
	}

	if (tchan_) {
		int n;
		double t = Scheduler::instance().clock();
		// XXX: be compatible with nsv1 RED trace entries
		if (strstr(v->name(), "curq") != NULL) {
			sprintf(wrk, "Q %g %d", t, int(*((TracedInt*) v)));
		} else {
			sprintf(wrk, "%c %g %g", *p, t,
				double(*((TracedDouble*) v)));
		}
		n = strlen(wrk);
		wrk[n] = '\n'; 
		wrk[n+1] = 0;
		(void)Tcl_Write(tchan_, wrk, n+1);
	}
	return; 
}

/* for debugging help */
void REDQueue::print_edp()
{
	printf("mean_pktsz: %d\n", edp_.mean_pktsize); 
	printf("bytes: %d, wait: %d, setbit: %d\n",
		edp_.bytes, edp_.wait, edp_.setbit);
	printf("minth: %f, maxth: %f\n", edp_.th_min, edp_.th_max);
	printf("max_p: %f, qw: %f, ptc: %f\n",
		(double) edv_.cur_max_p, edp_.q_w, edp_.ptc);
	printf("qlim: %d, idletime: %f\n", qlim_, idletime_);
	printf("mark_p: %f, use_mark_p: %d\n", edp_.mark_p, edp_.use_mark_p);
	printf("=========\n");
}

void REDQueue::print_edv()
{
	printf("v_a: %f, v_b: %f\n", edv_.v_a, edv_.v_b);
}

void REDQueue::print_summarystats()
{
	//double now = Scheduler::instance().clock();
	printf("True average queue: %5.3f", true_ave_);
	if (qib_) 
		printf(" (in bytes)");
        printf(" time: %5.3f\n", total_time_);
}

/************************************************************/
/*
 * This procedure is obsolete, and only included for backward compatibility.
 * The new procedure is REDQueue::estimator
 */ 
/*
 * Compute the average queue size.
 * The code contains two alternate methods for this, the plain EWMA
 * and the Holt-Winters method.
 * nqueued can be bytes or packets
 */
void REDQueue::run_estimator(int nqueued, int m)
{
	double f, f_sl;

	f = edv_.v_ave;
	f_sl = edv_.v_slope;
#define RED_EWMA
#ifdef RED_EWMA
	while (--m >= 1) {
		f *= 1.0 - edp_.q_w;
	}
#ifdef RED_HOLT_WINTERS
	double f_old;
	f_old = f;
#endif
	f *= 1.0 - edp_.q_w;
	f += edp_.q_w * nqueued;
#endif
#ifdef RED_HOLT_WINTERS
	while (--m >= 1) {
		f_old = f;
		f += f_sl;
		f *= 1.0 - edp_.q_w;
		f_sl *= 1.0 - 0.5 * edp_.q_w;
		f_sl += 0.5 * edp_.q_w * (f - f_old);
	}
	f_old = f;
	f += f_sl;
	f *= 1.0 - edp_.q_w;
	f += edp_.q_w * nqueued;
	f_sl *= 1.0 - 0.5 * edp_.q_w;
	f_sl += 0.5 * edp_.q_w * (f - f_old);
#endif
	edv_.v_ave = f;
	edv_.v_slope = f_sl;
}

void REDQueue::reportDrop(Packet *)
{}


//卫星链路的吞吐量
void REDQueue::recordPktcnt_allpertp(){
	double t = Scheduler::instance().clock();
	for(int i=0; i<82; i++){
		if(allnode_tp[i] >= 0){
			ofstream of;
			char txt[10];
			sprintf(txt, "(%d-%d)", sid, i);
			string s = txt;
			s = path+"perThroughput"+s+".txt";
			of.open(s.c_str(), ios::app);
			of << t << " " <<allnode_tp[i] << endl;
			of.close();
			allnode_tp[i] = 0;
		}
	}
}

//卫星链路的丢包率
void REDQueue::recordPktcnt_allflowperdp(double now){
	ifstream ifs;
	ofstream of;
	string data;

	for(int i= 0; i<3; i++){
		char txt[2];
		sprintf(txt, "%d",i);
		string s = txt;
		string s1 = path+"perFlowDropRate"+s+".txt";
		//cout << allflow_dp[i] << endl;
		if(allflow_dp[i] >= 0){
			of.open(s1.c_str(), ios::app);
			of << now << " " << allflow_dp[i] << endl;
			of.close();
			allflow_dp[i] = 0;
		}
	}
}

//记录地面链路吞吐量
void REDQueue::recordTp_gd(){
	double t = Scheduler::instance().clock();
	ofstream of;
	char txt[10];
	sprintf(txt, "%d-%d", sid, did);
	string s1 = txt;
	string s2 = path+"tp_gd"+s1+".txt";
	of.open(s2.c_str(), ios::app);
	// of << t << " " << countTp_gd << endl;
	of << countTp_gd << endl;
	countTp_gd = 0;
	of.close();
}

//记录地面链路丢包率
void REDQueue::recordDr_gd(){
	double t = Scheduler::instance().clock();
	ofstream of;
	char txt[10];
	sprintf(txt, "%d-%d", sid, did);
	string s1 = txt;
	string s2 = path+"dr_gd"+s1+".txt";
	of.open(s2.c_str(), ios::app);
	//of << t << " " << countDr_gd << endl;
	of << countDr_gd << endl;
	countDr_gd = 0;
	of.close();
}
