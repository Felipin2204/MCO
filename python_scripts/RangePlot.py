import sys
from omnetpp.scave.results import *
from matplotlib import pyplot as plt


if len(sys.argv) == 2:
     
    scenarios = (1, 2, 3, 4, 5)
    if (sys.argv[1] == '23dBm'):
        range_values = {
            'Only half of the vehicles in one channel': (1100, 800, 500, 200, 0),
            'Half-half in two adjacent channels:': (1000, 700, 400, 100, 0),
            'Half-half in channels separated by an empty channel': (1000, 800, 500, 200, 0),
            'All nodes in one channel': (800, 500, 0, 0, 0)
        }
    else:
        range_values = {
            'Only half of the vehicles in one channel': (1900, 1300, 300, 0, 0),
            'Half-half in two adjacent channels:': (1600, 900, 200, 0, 0),
            'Half-half in channels separated by an empty channel': (1800, 1200, 300, 0, 0),
            'All nodes in one channel': (1300, 300, 0, 0, 0)
        }
        
    x = np.arange(len(scenarios))
    colors = ['blue', 'red', 'yellow', 'purple']
    width = 0.2
    multiplier = 0
    
    fig, ax = plt.subplots()
    
    for attribute, measurement in range_values.items():
        offset = width * multiplier
        rects = ax.bar(x + offset, measurement, width=0.17, label=attribute, edgecolor='black', color=colors[multiplier])
        multiplier += 1
    
    ax.set_title('Range (PRR=0.9), '+sys.argv[1], fontweight='bold')
    
    ax.set_xlabel('Scenario')
    ax.set_xticks(x + 1.5*width, scenarios)
    
    ax.set_ylabel('Range [m]')
    ax.set_ylim(0, 3000)
    
    ax.grid(True)
    ax.set_axisbelow(True)
    ax.legend(loc='upper right', fontsize='small')
    
    plt.savefig('Range'+sys.argv[1]+'.png')
    
else:
    print("Error - The number of arguments is invalid")
    