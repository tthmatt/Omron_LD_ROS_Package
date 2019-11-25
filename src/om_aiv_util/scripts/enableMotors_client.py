#!/usr/bin/env python
import sys
import rospy
from om_aiv_util.srv import *
def enableMotors_client(x):
    rospy.wait_for_service('enableMotors')
    try:
        service = rospy.ServiceProxy('enableMotors', Service)
        resp1 = service(x)
        return resp1.device
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

def usage():
    return "%s"%sys.argv[0]

if __name__ == "__main__":
    if len(sys.argv) == 1:
        x = ""

    else:
        print usage()
        sys.exit(1)
    print "running command"
    print enableMotors_client(x)
