from scapy.utils import RawPcapReader
print("Importing ether")
from scapy.layers.l2 import Ether
print("Imported ether")
from scapy.layers.inet import IP, TCP

def process_pcap(file_name):
	print('Opening {}...'.format(file_name))

	count = 0
	for (pkt_data, pkt_metadata) in RawPcapReader(file_name):
		count += 1



		print("##################     ", count)

		ether_pkt = Ether(pkt_data)

		ip_pkt = ether_pkt[IP]
        
		if ip_pkt.proto != 6:
			# Ignore non-TCP packet
			continue

		tcp_pkt = ip_pkt[TCP]
		
		tcp_payload_len = ip_pkt.len - (ip_pkt.ihl * 4) - (tcp_pkt.dataofs * 4)
		
		print(tcp_pkt, tcp_payload_len, tcp_pkt.flags)

		print("##################")

		if count == 10:
			break

	print('{} contains {} packets'.format(file_name, count))

process_pcap("/Volumes/TimeCapsule/FFS/run1/ffs-trace.pcapng")