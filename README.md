# iot-i-am-here
use a PIR motion sensor to detect if someone in the office cubicle or not.

## IoT Topology

	{IamHere_Sender}            {ThingSpeak}            {IamHere_Receiver}
	 |-- PIR-sensor              |-- widgets              |-- Button
	 `-- ESP-01     >>> push >>> `-- chart   <<< get <<<  `-- ESP-01


## IamHere_Sender


## IamHere_Receiver

