# IoT: I-am-Here

use a PIR motion sensor to detect if someone in the office cubicle or not.

* [中文說明](README.zh_TW.md)


## IoT Topology

	{IamHere_Sender}            {ThingSpeak}            {IamHere_Receiver}
	 |-- PIR-sensor              |-- widgets              |-- Button
	 `-- ESP-01     >>> push >>> `-- chart   <<< get <<<  `-- ESP-01


## IamHere_Sender


## IamHere_Receiver

