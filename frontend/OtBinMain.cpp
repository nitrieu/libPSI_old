#include "bloomFilterMain.h"

#include "MPSI/Beta/OtBinMPsiReceiver.h"
#include "MPSI/Beta/OtBinMPsiSender.h"

#include <fstream>
using namespace osuCrypto;
#include "util.h"

#include "Common/Defines.h"
#include "Network/BtEndpoint.h" 
#include "NChooseOne/KkrtNcoOtReceiver.h"
#include "NChooseOne/KkrtNcoOtSender.h"

#include "NChooseOne/Oos/OosNcoOtReceiver.h"
#include "NChooseOne/Oos/OosNcoOtSender.h"
#include "Common/Log.h"
#include "Common/Timer.h"
#include "Crypto/PRNG.h"
#include <numeric>

#define OOS
#define pows  { 16/*8,12,,20*/ }
#define threadss {1/*1,4,16,64*/}

void otBinSend()
{


    Log::setThreadName("CP_Test_Thread");
    u64 numThreads(64);

    std::fstream online, offline;
    online.open("./online.txt", online.trunc | online.out);
    offline.open("./offline.txt", offline.trunc | offline.out);
    u64 numTrial(2);


    Log::out << "role  = sender (" << numThreads << ") otBin" << Log::endl;

    std::string name("psi");

    BtIOService ios(0);
    BtEndpoint sendEP(ios, "localhost", 1213, true, name);

    std::vector<Channel*> sendChls_(numThreads);

    for (u64 i = 0; i < numThreads; ++i)
    {
        sendChls_[i] = &sendEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
    }
    u8 dummy[1];

    senderGetLatency(*sendChls_[0]);
    sendChls_[0]->resetStats();

    LinearCode code;
    code.loadBinFile(SOLUTION_DIR "../libOTe/libOTe/Tools/bch511.bin");

    //for (auto pow : {/* 8,12,*/ 16/*, 20 */ })
    for (auto pow : pows)
    {

        for (auto cc : threadss)
        {
            std::vector<Channel*> sendChls;

            if (pow == 8)
                cc = std::min(8, cc);

            //Log::out << "numTHreads = " << cc << Log::endl;

            sendChls.insert(sendChls.begin(), sendChls_.begin(), sendChls_.begin() + cc);

            u64 offlineTimeTot(0);
            u64 onlineTimeTot(0);
            //for (u64 numThreads = 1; numThreads < 129; numThreads *= 2)
            for (u64 jj = 0; jj < numTrial; jj++)
            {

                //u64 repeatCount = 1;
                u64 setSize = (1 << pow), psiSecParam = 40;
                PRNG prng(_mm_set_epi32(4253465, 3434565, 234435, 23987045));


                std::vector<block> sendSet;
                sendSet.resize(setSize);

                for (u64 i = 0; i < setSize; ++i)
                {
                    sendSet[i] = prng.get<block>();
                }

#ifdef OOS
                OosNcoOtReceiver otRecv(code);
                OosNcoOtSender otSend(code);
#else
                KkrtNcoOtReceiver otRecv;
                KkrtNcoOtSender otSend;
#endif
                OtBinMPsiSender sendPSIs;

                //gTimer.reset();

                sendChls[0]->asyncSend(dummy, 1);
                sendChls[0]->recv(dummy, 1);
                u64 otIdx = 0;
                //Log::out << "sender init" << Log::endl;
                sendPSIs.init(setSize, psiSecParam,128, sendChls,otSend, otRecv, prng.get<block>());

                //return;
                sendChls[0]->asyncSend(dummy, 1);
                sendChls[0]->recv(dummy, 1);
                //Log::out << "sender init done" << Log::endl;

                sendPSIs.sendInput(sendSet, sendChls);

                u64 dataSent = 0;
                for (u64 g = 0; g < sendChls.size(); ++g)
                {
                    dataSent += sendChls[g]->getTotalDataSent();
                }

                //std::accumulate(sendChls[0]->getTotalDataSent())

                //Log::out << setSize << "    " << dataSent / std::pow(2, 20) << " byte  " << Log::endl;
                for (u64 g = 0; g < sendChls.size(); ++g)
                    sendChls[g]->resetStats();

                //Log::out << gTimer << Log::endl;
            }

        }


    }
    for (u64 i = 0; i < numThreads; ++i)
    {
        sendChls_[i]->close();// = &sendEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
    }
    //sendChl.close();
    //recvChl.close();

    sendEP.stop();

    ios.stop();
}

void otBinRecv()
{

    Log::setThreadName("CP_Test_Thread");
    u64 numThreads(64);

    std::fstream online, offline;
    online.open("./online.txt", online.trunc | online.out);
    offline.open("./offline.txt", offline.trunc | offline.out);
    u64 numTrial(2);

    std::string name("psi");

    BtIOService ios(0);
    BtEndpoint recvEP(ios, "localhost", 1213, false, name);

    LinearCode code; 

    code.loadBinFile(SOLUTION_DIR "/../libOTe/libOTe/Tools/bch511.bin");

    std::vector<Channel*> recvChls_(numThreads);
    for (u64 i = 0; i < numThreads; ++i)
    {
        recvChls_[i] = &recvEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
    }

    Log::out << "role  = recv(" << numThreads << ") otBin" << Log::endl;
    u8 dummy[1];
    recverGetLatency(*recvChls_[0]);

    //for (auto pow : {/* 8,12,*/16/*,20*/ })
    for (auto pow : pows)
    {
        for (auto cc : threadss)
        {
            std::vector<Channel*> recvChls;

            if (pow == 8)
                cc = std::min(8, cc);

            u64 setSize = (1 << pow), psiSecParam = 40;

            Log::out << "numTHreads = " << cc << "  n=" << setSize << Log::endl;

            recvChls.insert(recvChls.begin(), recvChls_.begin(), recvChls_.begin() + cc);

            u64 offlineTimeTot(0);
            u64 onlineTimeTot(0);
            //for (u64 numThreads = 1; numThreads < 129; numThreads *= 2)
            for (u64 jj = 0; jj < numTrial; jj++)
            {

                //u64 repeatCount = 1;
                PRNG prng(_mm_set_epi32(4253465, 3434565, 234435, 23987045));


                std::vector<block> sendSet(setSize), recvSet(setSize);




                for (u64 i = 0; i < setSize; ++i)
                {
                    sendSet[i] = recvSet[i] = prng.get<block>();
                }


#ifdef OOS
                OosNcoOtReceiver otRecv(code);
                OosNcoOtSender otSend(code);
#else
                KkrtNcoOtReceiver otRecv;
                KkrtNcoOtSender otSend;
#endif
                OtBinMPsiReceiver recvPSIs;


                recvChls[0]->recv(dummy, 1);
                gTimer.reset();
                recvChls[0]->asyncSend(dummy, 1);

                u64 otIdx = 0;


                Timer timer;
                auto start = timer.setTimePoint("start");
                recvPSIs.init(setSize, psiSecParam,128,  recvChls, otRecv, otSend, ZeroBlock);
                //return;


                //std::vector<u64> sss(recvChls.size());
                //for (u64 g = 0; g < recvChls.size(); ++g)
                //{
                //    sss[g] =  recvChls[g]->getTotalDataSent();
                //}

                recvChls[0]->asyncSend(dummy, 1);
                recvChls[0]->recv(dummy, 1);
                auto mid = timer.setTimePoint("init");


                recvPSIs.sendInput(recvSet, recvChls);
                auto end = timer.setTimePoint("done");

                auto offlineTime = std::chrono::duration_cast<std::chrono::milliseconds>(mid - start).count();
                auto onlineTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - mid).count();


                offlineTimeTot += offlineTime;
                onlineTimeTot += onlineTime;
                //auto byteSent = recvChls[0]->getTotalDataSent() *recvChls.size();

                u64 dataSent = 0;
                for (u64 g = 0; g < recvChls.size(); ++g)
                {
                    dataSent += recvChls[g]->getTotalDataSent();
                    //Log::out << "chl[" << g << "] " << recvChls[g]->getTotalDataSent() << "   " << sss[g] << Log::endl;
                }

                double time = offlineTime + onlineTime;
                time /= 1000;
                auto Mbps = dataSent * 8 / time / (1 << 20);

                Log::out << setSize << "  " << offlineTime << "  " << onlineTime << "        " << Mbps << " Mbps      " << (dataSent / std::pow(2.0, 20)) << " MB" << Log::endl;

                for (u64 g = 0; g < recvChls.size(); ++g)
                    recvChls[g]->resetStats();

                //Log::out << "threads =  " << numThreads << Log::endl << timer << Log::endl << Log::endl << Log::endl;


                //Log::out << numThreads << Log::endl;
                //Log::out << timer << Log::endl;

                Log::out << gTimer << Log::endl;

                //if (recv.mIntersection.size() != setSize)
                //    throw std::runtime_error("");







            }



            online << onlineTimeTot / numTrial << "-";
            offline << offlineTimeTot / numTrial << "-";

        }
    }

    for (u64 i = 0; i < numThreads; ++i)
    {
        recvChls_[i]->close();// = &recvEP.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
    }
    //sendChl.close();
    //recvChl.close();

    recvEP.stop();

    ios.stop();
}

