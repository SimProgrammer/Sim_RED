
Queue/NoQue set pertime_ 1
Queue/NoQue set isSat_ 0
Queue/RED set thresh_ 20 ;#平均队列大小的最小阈值
Queue/RED set maxthresh_ 80 ;#平均队列大小的最大阈值
Queue/RED set q_weight_ 0.0027 ;#权值wq
Queue/RED set top_ 0.1 ;#最大丢弃概率
Queue/RED set mean_pktsize_ 1000
Queue/RED set pertime_ 1;#统计间隔
Queue/RED set nared_ 1;
Queue/RED set isSat_ 0; #关闭卫星模式
set num 30
#创建事件调度器和节点
set ns [new Simulator]

#创建输出文件并链接
set nf [open glevel.nam w]
#$ns namtrace-all $nf


proc finish {} {
	global ns nf
	$ns flush-trace
	close $nf

	#打开动画
	#exec nam glevel.nam &

	exit 0
}

#节点设置
set recv1 [$ns node]
set recv2 [$ns node]
set r1 [$ns node]
set r2 [$ns node]
set send1 [$ns node]
set send2 [$ns node]
set td1 [$ns node]
set td2 [$ns node]

#节点连接
$ns duplex-link $send1 $r1 10Mb 5ms DropTail
$ns duplex-link $send2 $r1 10Mb 5ms DropTail
$ns duplex-link $r1 $r2 4Mb 20ms RED
$ns queue-limit $r1 $r2 100
$ns queue-limit $r2 $r1 100
$ns duplex-link $r2 $recv1 10Mb 5ms DropTail
$ns duplex-link $r2 $recv2 10Mb 5ms DropTail
$ns duplex-link $recv1 $td1 1000Mb 5ms NoQue
$ns duplex-link $recv2 $td2 1000Mb 5ms NoQue



set tcp0 [new Agent/UDP]
$tcp0 set packetSize_ 1000
$ns attach-agent $send1 $tcp0
set end0 [new Agent/Null]
$ns attach-agent $td1 $end0
$ns connect $tcp0 $end0

set tcp1 [new Agent/UDP]
$tcp0 set packetSize_ 1000
$ns attach-agent $send2 $tcp1
set end1 [new Agent/Null]
$ns attach-agent $td2 $end1
$ns connect $tcp1 $end1



for {set i 0} {$i < $num} {incr i} {
    set pa [new Application/Traffic/MyPareto]
    $pa set packetSize_ 1000
    $pa set burst_time_ 320ms
    $pa set idle_time_ 1197ms
    $pa set rate_ 600Kb
    $pa set shape_ 1.6
    $pa attach-agent $tcp0
    $ns at 0.0 "$pa start"

    set pa2 [new Application/Traffic/MyPareto]
    $pa2 set packetSize_ 1000
    $pa2 set burst_time_ 320ms
    $pa2 set idle_time_ 1197ms
    $pa2 set rate_ 600Kb
    $pa2 set shape_ 1.6
    $pa2 attach-agent $tcp1
    $ns at 0.0 "$pa2 start"
}
    # set pa [new Application/Traffic/MyPareto]
    # $pa set packetSize_ 1000
    # $pa set burst_time_ 320ms
    # $pa set idle_time_ 1197ms
    # $pa set rate_ 600Kb
    # $pa set shape_ 1.6
    # $pa attach-agent $tcp0
    # $ns at 0.0 "$pa start"


$ns at 1000 "finish"
$ns run











