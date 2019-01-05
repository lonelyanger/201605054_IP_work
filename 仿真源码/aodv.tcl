set val(chan) Channel/WirelessChannel
set val(prop) Propagation/TwoRayGround
set val(netif) Phy/WirelessPhy
set val(mac) Mac/802_11
set val(ifq) Queue/DropTail/PriQueue
set val(ll) LL
set val(ant) Antenna/OmniAntenna
set val(ifqlen) 50
set val(nn) 50
set val(rp) AODV
set opt(cp) "cbr1"
set opt(sc) "scence1"


puts "\n"
puts "Simulation of a simple wireless topology running with AODV\n"
puts "Starting simulation...\n"


set ns_ [new Simulator]
set tracefd [open aodv.tr w]
$ns_ use-newtrace 
$ns_ trace-all $tracefd

set namtracefd [open aodv.nam w]
$ns_ namtrace-all-wireless $namtracefd 1000 300

set topo [new Topography] 
$topo load_flatgrid 1000 300
set god_ [new God] 
create-god $val(nn)

 

$ns_ node-config -adhocRouting $val(rp) \
-llType $val(ll) \
-macType $val(mac) \
-ifqType $val(ifq) \
-ifqLen $val(ifqlen) \
-antType $val(ant) \
-propType $val(prop) \
-phyType $val(netif) \
-channelType $val(chan) \
-topoInstance $topo \
-agentTrace ON \
-routerTrace ON \
-macTrace OFF \
-movementTrace OFF \

for {set i 0} {$i < $val(nn)} {incr i} {
set node_($i) [$ns_ node]
$node_($i) random-motion 0 
}


puts "Loading connection pattern file\n"
source $opt(cp)
puts "Connection pattern file loading complete...\n"


puts "Loading scenario file...\n"
source $opt(sc)
puts "Scenario file loading complete...\n"
puts "Simulation may take a few minutes...\n"
puts "A sample script runs"

#设置在nam中移动节点显示的大小，否则，nam中无法显示节点

for {set i 0} {$i < $val(nn)} {incr i} {

$ns_ initial_node_pos $node_($i) 20

}
for {set i 0} {$i < $val(nn)} {incr i} {
$ns_ at 300.1 "$node_($i) reset"
}
$ns_ at 300.2 "stop"
$ns_ at 300.3 "puts\”Simulation runs sucessfully and NS exiting…\"; $ns_ halt"
proc stop {} {
global ns_ tracefd namtracefd
$ns_ flush-trace
close $tracefd
close $namtracefd
exec nam aodv.nam &
exit 0
}

puts $tracefd "Here is a trace for simple wireless simulation\n"
puts $tracefd "The nodes movement file is $opt(cp)\n"
puts $tracefd "The traffic flow between nodes is $opt(sc)\n"
$ns_ run

