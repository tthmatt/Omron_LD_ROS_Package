#!/usr/bin/env python
from socketconnection_class import ConnectSocket, connecttcp
s = connecttcp.sock
import socket
import threading
import time
import re
import sys
import rospy
from std_msgs.msg import String
BUFFER_SIZE = 1024
ip_address = rospy.get_param("ip_address")
port = rospy.get_param("port")
# ip_address = "172.21.5.125"
# port = 7171
connecttcp.connect(str(ip_address), port)

from om_aiv_util.srv import Service5,Service5Response
import rospy

def handle_applicationFaultSet(req):
    global a, b, c, d, e
    a = req.a
    b = req.b
    c = req.c
    d = req.d
    e = req.e
    applicationFaultSet()
    return rcv

def applicationFaultSet_server():
    rospy.init_node('applicationFaultSet_server')
    s = rospy.Service('applicationFaultSet', Service5, handle_applicationFaultSet)
    rospy.spin()

def applicationFaultSet():
    global rcv
    pub = rospy.Publisher('arcl_applicationFaultSet', String, queue_size=10)
    rate = rospy.Rate(10) # 10hz
    command = "applicationFaultSet {}".format(a + " " + b + " " + c + " " + d + " " + e)
    command = command.encode('ascii')
    print "Running command: ", command
    s.send(command+b"\r\n")
    try:
        data = s.recv(BUFFER_SIZE)
        rcv = data.encode('ascii', 'ignore')
        while not rospy.is_shutdown():
            #check for required data
            if "Fault:" in rcv:
                print rcv
                return rcv
                break
            if "CommandErrorDescription" in rcv:
                print rcv
                return rcv
                break
            else:
                data = s.recv(BUFFER_SIZE)
                rcv = rcv + data.encode('ascii', 'ignore')

    except socket.error as errormsg:
        print("Connection  failed")
        return errormsg

if __name__ == "__main__":
    applicationFaultSet_server()
