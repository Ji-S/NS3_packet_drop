# NS3_packet_drop

환경 설정 및 tutorial 참조 링크


https://www.nsnam.org/docs/release/3.30/tutorial/singlehtml/index.html
https://www.nsnam.org/docs/release/3.30/tutorial/ns-3-tutorial.pdf

====Parameters====
Error Rate: 0.001%(default) 	-> bit error, byte error, packet error 3개 선택가능.
Link env : Point to Point(default), WiFi, p2p and csma ….
Channel Data Rate: 5Mbps(default)
Channel Delay: 2ms(default)
Application Data Rate: 1Mbps(default)
Packet size : 1040(default)		-> 1500 변경가능. (header 54 제외 payload: 1446)
Packet transmission cycle: 8 * packet size / Application Data Rate. Get Bit Rate (default)
			ex) 0.00832   	8*KB/bit 단위 통일
MTU : 1500 (default) 		-> 변경 가능
Segment Size : 536(default)  : based on MTU -> 변경 가능 (3000)
Simulation Time : 10(default) : data rate에 따라 모든 data 전송 할 수도 있고 못 할 수도 있음.


