import sys
from omnetpp.scave.results import *
from matplotlib import pyplot as plt


if len(sys.argv) == 2: 
     
    densities = (12.5, 25, 50, 75, 100)
    nodes = [int(i*8) for i in densities]
    
    irt_values = np.zeros((4,5,5))
        
    for i in range(4):
        for j in range(5):
            for k in range(5):
            
                filename="density_"+str(densities[j])+"/Balanced_Traffic_Density_"+str(densities[j])+"-nodes="+str(nodes[j]//2*(1+(i%4!=0)))
                if j==0: filename="density_12,5/Balanced_Traffic_Density_12_5-nodes="+str(nodes[j]//2*(1+(i%4!=0)))
                if sys.argv[1]=='23dBm': filename+=",channelNumber="+str(i%3)+",adjacentLoss=25.9_40.6,txPower=0.1W-#"+str(k)
                else: filename+=",channelNumber="+str(i%3)+",adjacentLoss=32.1_46.4,txPower=1W-#"+str(k)
        
                #Vector values
                set_inputs(filename + ".vec")
                
                #IRT
                aux=get_vectors("module=~TraficGeneratorNetwork.node[.."+str(nodes[j]//2-1)+"].MCO.vehicleTable AND name=~irt:vector")["vecvalue"].to_numpy()
                irt_values[i][j][k]=np.percentile(np.concatenate(aux), 99)
                print(str((i*25)+(j*5)+k+1)+"% Completed")
    
    irt_names = ('Only half of the vehicles in one channel', 'Half-half in two adjacent channels:',
                 'Half-half in channels separated by an empty channel', 'All nodes in one channel')
    irt_means = {
        'Only half of the vehicles in one channel': np.zeros(5),
        'Half-half in two adjacent channels:': np.zeros(5),
        'Half-half in channels separated by an empty channel': np.zeros(5),
        'All nodes in one channel': np.zeros(5)
    }
    
    for i in range(4):
        irt_means[irt_names[i]] = np.mean(irt_values, axis=2)[i]
    
    irt_stds = np.std(irt_values, axis=2)
    
    scenarios = (1, 2, 3, 4, 5)
    x = np.arange(len(scenarios))
    colors = ['blue', 'red', 'yellow', 'purple']
    width = 0.2
    multiplier = 0
    
    fig, ax = plt.subplots()
    
    for attribute, measurement in irt_means.items():
        offset = width * multiplier
        rects = ax.bar(x + offset, measurement, yerr=irt_stds[multiplier], width=0.17, label=attribute, edgecolor='black', color=colors[multiplier], capsize=3)
        multiplier += 1
    
    ax.set_title('ccdf IPG 0.01, '+sys.argv[1], fontweight='bold')
    
    ax.set_xlabel('Scenario')
    ax.set_xticks(x + 1.5*width, scenarios)
    
    ax.set_ylabel('ccdf IPG 0.01')
    ax.set_ylim(5*10**-2, 2*10**0)
    ax.set_yscale('log')
    
    ax.grid(True, which="major")
    ax.grid(True, which="minor", linestyle="dotted")
    ax.set_axisbelow(True)
    ax.legend(loc='upper left', fontsize='small')
    
    plt.savefig('IPG'+sys.argv[1]+'.png')
    
else:
    print("Error - The number of arguments is invalid")
    