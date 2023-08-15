import sys
from omnetpp.scave.results import *
from matplotlib import pyplot as plt
import scipy.stats


if len(sys.argv) == 3: 
     
    results=np.zeros((4,5,30))
    
    for i in range(4):
        
        for k in range(5):
        
            filename="Balanced_Traffic_Density_"+sys.argv[1]+"-nodes="+str(int(float(sys.argv[1])*8)//2*(1+(i%4!=0)))
            if sys.argv[1]=='12.5': filename="Balanced_Traffic_Density_12_5-nodes="+str(int(float(sys.argv[1])*8)//2*(1+(i%4!=0)))
            if sys.argv[2]=='23dBm': filename+=",channelNumber="+str(i%3)+",adjacentLoss=25.9_40.6,txPower=0.1W-#"+str(k)
            else: filename+=",channelNumber="+str(i%3)+",adjacentLoss=32.1_46.4,txPower=1W-#"+str(k)
            
            #Vector values
            set_inputs(filename + ".vec")
            
            #PDR
            for j in range(30):
                aux=get_vectors("module=~TraficGeneratorNetwork.node[.."+str((int(float(sys.argv[1])*8)//2)-1)+"].MCO.mgmt AND name=~pdr" + str(j*100) + "-" + str((j+1)*100) + ":vector")["vecvalue"].to_numpy()
                pdr=np.concatenate(aux)
                results[i][k][j]=pdr.mean()
    
    #Plotting PDR 
    fig,ax=plt.subplots()
    scenario={'12.5':1,'25':2,'50':3,'75':4,'100':5}
    speed={'12.5':130,'25':120,'50':110,'75':100,'100':80}
    ax.set_title("PRR, scenario "+str(scenario[sys.argv[1]])+": "+str(sys.argv[1])+"v/km @"+str(speed[sys.argv[1]])+"km/h, EIRP "+sys.argv[2], fontweight='bold')
    ax.set_xlabel("Distance [m]")
    ax.set_ylabel("PRR")
    ax.axis([0, 3000, 0, 1])
    ax.set_yticks(np.linspace(0,1,11))
    ax.grid(True)
    
    #Plots
    x=np.arange(100, 3100, 100)
    
    PRR90='@'+str((np.where(np.mean(results, axis=1)[0]>=0.9)[0][-1]+1)*100)+'m' if np.where(np.mean(results, axis=1)[0]>=0.9)[0].size!=0 else 'n.a.'
    ax.plot(x, np.mean(results, axis=1)[0], color='blue', label='Only half of the vehicles in one channel, 90% PRR '+PRR90)
    se = scipy.stats.sem(results[0])
    ci = se * scipy.stats.t.ppf((1 + 0.95) / 2., 4)
    ax.fill_between(x, (np.mean(results, axis=1)[0]-ci), (np.mean(results, axis=1)[0]+ci), color='blue', alpha=.3)
    
    PRR90='@'+str((np.where(np.mean(results, axis=1)[1]>=0.9)[0][-1]+1)*100)+'m' if np.where(np.mean(results, axis=1)[1]>=0.9)[0].size!=0 else 'n.a.'
    ax.plot(x, np.mean(results, axis=1)[1], color='red', label='Half-half in two adjacent channels, 90% PRR '+PRR90)
    se = scipy.stats.sem(results[1])
    ci = se * scipy.stats.t.ppf((1 + 0.95) / 2., 4)
    ax.fill_between(x, (np.mean(results, axis=1)[1]-ci), (np.mean(results, axis=1)[1]+ci), color='red', alpha=.3)
    
    PRR90='@'+str((np.where(np.mean(results, axis=1)[2]>=0.9)[0][-1]+1)*100)+'m' if np.where(np.mean(results, axis=1)[2]>=0.9)[0].size!=0 else 'n.a.'
    ax.plot(x, np.mean(results, axis=1)[2], color='yellow', label='Half-half in channels separated by an empty channel, 90% PRR '+PRR90)
    se = scipy.stats.sem(results[2])
    ci = se * scipy.stats.t.ppf((1 + 0.95) / 2., 4)
    ax.fill_between(x, (np.mean(results, axis=1)[2]-ci), (np.mean(results, axis=1)[2]+ci), color='yellow', alpha=.3)
    
    PRR90='@'+str((np.where(np.mean(results, axis=1)[3]>=0.9)[0][-1]+1)*100)+'m' if np.where(np.mean(results, axis=1)[3]>=0.9)[0].size!=0 else 'n.a.'
    ax.plot(x, np.mean(results, axis=1)[3], color='purple', linestyle='--', label='All nodes in one channel, 90% PRR '+PRR90)
    se = scipy.stats.sem(results[3])
    ci = se * scipy.stats.t.ppf((1 + 0.95) / 2., 4)
    ax.fill_between(x, (np.mean(results, axis=1)[3]-ci), (np.mean(results, axis=1)[3]+ci), color='purple', alpha=.3)
    
    ax.legend(loc='lower left', fontsize='small')
    plt.show()
    
else:
    print("Error - The number of arguments is invalid")
    