import sys
from omnetpp.scave.results import *


if len(sys.argv) == 3:
     
    results=np.zeros((8, 36))
    
    for i in range(8):
        
        results_repetition=np.zeros((5, 36))
        
        for k in range(5):
        
            filename="Balanced_Traffic_Density_"+sys.argv[1]+"-nodes="+str(int(float(sys.argv[1])*8)//2*(1+(i%4!=0)))
            if i<4: filename+=",channelNumber="+str(i%3)+",adjacentLoss=25.9_40.6,txPower=0.1W-#"+str(k)
            else: filename+=",channelNumber="+str(i%4%3)+",adjacentLoss=32.1_46.4,txPower=1W-#"+str(k)
            
            #Scalar values
            set_inputs(filename + ".sca")
            
            #Generated packets
            generatedPackets=get_scalars("module=~TraficGeneratorNetwork.node[.."+str((int(float(sys.argv[1])*8)//2)-1)+"].application[0] AND name=~generatedPackets:last")["value"].to_numpy()
            # print("Generated packets mean value:", generatedPackets.mean())
            results_repetition[k][0]=generatedPackets.mean()
            
            #Received packets
            receivedPackets=get_scalars("module=~TraficGeneratorNetwork.node[.."+str((int(float(sys.argv[1])*8)//2)-1)+"].application[0] AND name=~receivedPackets:last")["value"].to_numpy()
            # print("Received packets mean value:", receivedPackets.mean())
            results_repetition[k][1]=receivedPackets.mean()
            
            #Neighbors
            neighbors=get_scalars("module=~TraficGeneratorNetwork.node[.."+str((int(float(sys.argv[1])*8)//2)-1)+"].MCO.vehicleTable AND name=~neighbors:mean")["value"].to_numpy()
            # print("Neighbors mean value:", neighbors.mean())
            results_repetition[k][2]=neighbors.mean()
            
            #CBT
            cbt=get_scalars("module=~TraficGeneratorNetwork.node[.."+str((int(float(sys.argv[1])*8)//2)-1)+"].MCO.mgmt AND name=~cbt:mean")["value"].to_numpy()
            # print("CBT mean value:", cbt.mean())
            results_repetition[k][3]=cbt.mean()
            
            
            #Vector values
            set_inputs(filename + ".vec")
            
            #Time between packets
            aux=get_vectors("module=~TraficGeneratorNetwork.node[.."+str((int(float(sys.argv[1])*8)//2)-1)+"].application[0] AND name=~timeBetweenPackets:vector")["vecvalue"].to_numpy()
            timebetweenpackets=np.concatenate(aux)
            # print("Time between packets mean value:", timebetweenpackets.mean())
            results_repetition[k][4]=timebetweenpackets.mean()
            
            #IRT
            aux=get_vectors("module=~TraficGeneratorNetwork.node[.."+str((int(float(sys.argv[1])*8)//2)-1)+"].MCO.vehicleTable AND name=~irt:vector")["vecvalue"].to_numpy()
            irt=np.concatenate(aux)
            # print("IRT 0.01 CCDF value:", np.percentile(irt, 99))
            results_repetition[k][5]=np.percentile(irt, 99)
            
            #PDR
            for j in range(30):
                aux=get_vectors("module=~TraficGeneratorNetwork.node[.."+str((int(float(sys.argv[1])*8)//2)-1)+"].MCO.mgmt AND name=~pdr" + str(j*100) + "-" + str((j+1)*100) + ":vector")["vecvalue"].to_numpy()
                if aux.size != 0:
                    pdr=np.concatenate(aux)
                    # print("PDR " + str(j*100) + "-" + str((j+1)*100) + " mean value:", pdr.mean())
                    results_repetition[k][j+6]=pdr.mean()
                else:
                    results_repetition[k][j+6]=0
                    
        results[i]=np.mean(results_repetition, axis=0)
        print(i)
                    
    aux='Generated packets mean,Received packets mean,Neighbors mean,CBT mean,Time between packets mean,IRT 0.01 CCDF'
    for i in range(30):
        aux+= ',PDR ' + str(i*100) + '-' + str((i+1)*100) + ' mean'
    
    np.savetxt(sys.argv[2], results, delimiter=',', header=aux)
    
else:
    print("Error - The number of arguments is invalid")
    