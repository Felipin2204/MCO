import sys
from omnetpp.scave.results import *
from matplotlib import pyplot as plt

#First argument - Input: Name of the data file (without the extension)
#Second argument - Output: Name of the results file (without the extension)

if len(sys.argv) == 3: 
    
    #Set of simulation result files to use as input
    set_inputs([sys.argv[1] + ".sca", sys.argv[1] + ".vec"])
    
    #Getting the number of channels used in the simulation
    numChannels=int(get_parameters("module=~TraficGeneratorNetwork.node[0] AND name=~numChannels")["value"].pop(0))
    
    #PDR
    matrixPDR=np.zeros((numChannels,20))
    for i in range(numChannels):
        for j in range(20):
            aux=get_vectors("name=~pdr" + str(i) + "_" + str(j*100) + "-" + str((j+1)*100) + ":vector")["vecvalue"].to_numpy()
            if aux.size != 0:
                pdr=np.concatenate(aux)
                matrixPDR[i][j]=pdr.mean()
            else:
                matrixPDR[i][j]=0
        print('PDR channel ' + str(i) + ' processed')
        
    #Plotting PDR
    fig,ax=plt.subplots()

    #Getting some parameters values for the figure title
    numberNodes=int(get_parameters("name=~nodes")["value"].pop(0))
    typenameMCO=get_parameters("module=~TraficGeneratorNetwork.node[0].MCO AND name=~typename")["value"].pop(0)
    typenameMCO=typenameMCO.split(".")[-1][:-1]
    numApplications=int(get_parameters("module=~TraficGeneratorNetwork.node[0] AND name=~numApplications")["value"].pop(0))

    ax.set_title("PRR, "+typenameMCO+", nodes="+str(numberNodes)+", apps="+str(numApplications), fontweight='bold')
    ax.set_xlabel("Distance [m]")
    ax.set_ylabel("PRR")
    ax.axis([0, 2000, 0, 1])
    ax.set_yticks(np.linspace(0,1,11))
    ax.grid(True)

    #Plots
    x=np.arange(100, 2100, 100)

    for i in range(numChannels):
        ax.plot(x, matrixPDR[i], label='Channel '+str(i))

    ax.legend(loc='lower left', fontsize='x-small')
    plt.savefig(sys.argv[2] + '.png')

else:
    print("Error - The number of arguments is invalid")
    