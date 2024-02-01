import sys
from omnetpp.scave.results import *
from matplotlib import pyplot as plt

#First argument - Input: Name of the data file (without the extension)
#Second argument - Output: Name of the results file (without the extension)

if len(sys.argv) == 3: 
    
    #Vector values
    set_inputs(sys.argv[1] + ".vec")
    
    #PDR
    matrixPDR=np.zeros((7,20))
    for i in range(7):
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

    #Getting some parameters values
    set_inputs(sys.argv[1] + ".sca")
    numberNodes=get_parameters("name=~nodes")["value"].pop(0)
    typenameMCO=get_parameters("module=~TraficGeneratorNetwork.node[0].MCO AND name=~typename")["value"].pop(0)
    typenameMCO=typenameMCO.split(".")[-1][:-1]

    ax.set_title("PRR vs. distance, "+typenameMCO+", nodes="+str(numberNodes), fontweight='bold')
    ax.set_xlabel("Distance [m]")
    ax.set_ylabel("PRR")
    ax.axis([0, 2000, 0, 1])
    ax.set_yticks(np.linspace(0,1,11))
    ax.grid(True)

    #Plots
    x=np.arange(100, 2100, 100)

    for i in range(7):
        ax.plot(x, matrixPDR[i], label='Channel '+str(i))

    ax.legend(loc='lower left', fontsize='x-small')
    plt.savefig(sys.argv[2] + '.png')

else:
    print("Error - The number of arguments is invalid")
    