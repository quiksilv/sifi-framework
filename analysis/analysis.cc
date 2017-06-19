// STL includes
#include <vector>
#include <time.h>
#include <string.h>
#include <list>
#include <iostream>
#include <chrono>

#include <getopt.h>

// root includes
#include <TDatabasePDG.h>
#include <TVector3.h>
#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TH1.h>

// MAPT-Analysis framework includes
#include "Hits30x30.h"
#include "Event.h"
#include "EventSim.h"
#include "AnalysisData.h"
#include "B1Particle.hh"
#include "B1DetectorResponse.hh"
#include "DataManager.hh"

using namespace std;

int main(int argc,char** argv)
{
    // OPEN AND WRITE
    DataManager dataManager;

    dataManager.setOpenTreeName("TreeName");
//     dataManager.open("test.root");

    int c;
    while(1)
    {
        static struct option long_options[] = {
            { 0, 0, 0, 0 }
        };

        int option_index = 0;

        c = getopt_long(argc, argv, "", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            default:
                break;
        }
    }

    if (optind < argc)
    {
        while (optind < argc)
        {
            dataManager.open(argv[optind]);
            ++optind;
            break;
        }
    }

    dataManager.book("stoppInDetector.root");

    // // load data from root
    // // create string with file path
	// string path = "/nfs/hicran/project/e18sat/analysis/newGeometry/PENGeometry/analysis/preselectedDataSets/selectedIsoEvents50MeV.root";
	// // load root event files
	// TFile *f = TFile::Open(path.c_str());
	// if (f == 0){
	// 	printf("Error: cannot open ROOT file");
	// 	return -1;
	// }
	// TTree *tree = (TTree*) f->Get("TreeName");
	// B1DetectorResponse *response = new B1DetectorResponse();
	// B1Particle* incoming = 0;
	// tree->GetBranch("DetectorResponse")->SetAddress(&response);
	// tree->GetBranch("Incoming_Particle")->SetAddress(&incoming);


    // DataManager dataManager;

    // WRITE EVENTS

    // dataManager.book("testing.root");
    //
    // for (int i=0;i<400;i++)
    // {
    //     // get tree data
    //     tree->GetEntry(i);
    //
    //     Event* event = dataManager.getEvent();
    //
    //     event->setHits(response->energy_deposition);
    //     event->getSimulatedEvent()->setPrimary(incoming);
    //     event->getSimulatedEvent()->setDetectorResponse(response);
    //
    //     for (int i=0; i<1000;i++){
    //         event->getFitData()->fill(i,i,i,i,i,i,i);
    //     }
    //
    //     dataManager.fill();
    // }
    //
    // dataManager.save();


    // OPEN EVENTS

    // dataManager.open("testing.root");
    // int entries = dataManager.getEntriesFast();
    // dataManager.getEntry(302);
    // std::cout << entries << "\n";
    //
    // dataManager.getEvent()->getHits()->print();
    // dataManager.getEvent()->getSimulatedEvent()->getPrimary()->Get_start_direction().Print();

    TH1I * h_pi_mult = new TH1I("h_pi_mult", "h_pi_mult", 20, 0, 20);
    TH1I * h_pim_mult = new TH1I("h_pim_mult", "h_pim_mult", 10, 0, 10);
    TH1I * h_pip_mult = new TH1I("h_pip_mult", "h_pip_mult", 10, 0, 10);
    TH1I * h_piz_mult = new TH1I("h_piz_mult", "h_piz_mult", 10, 0, 10);

    TH1I * h_pi_mult_1stgen = new TH1I("h_pi_mult_1stgen", "h_pi_mult_1stgen", 20, 0, 20);
    TH1I * h_pim_mult_1stgen = new TH1I("h_pim_mult_1stgen", "h_pim_mult_1stgen", 10, 0, 10);
    TH1I * h_pip_mult_1stgen = new TH1I("h_pip_mult_1stgen", "h_pip_mult_1stgen", 10, 0, 10);
    TH1I * h_piz_mult_1stgen = new TH1I("h_piz_mult_1stgen", "h_piz_mult_1stgen", 10, 0, 10);

    std::cout << dataManager.getEntriesFast() << " Events" << "\n";
    int secs = 0;
    for (int i=0 ; i < dataManager.getEntriesFast(); ++i)
    {
        dataManager.getEntry(i);
        Event* event = dataManager.getEvent();
        if (!event)
        {
            std::cerr << "event NULL" << "\n";
            return -1;
        }
//         std::cout << event->getSimulatedEvent()->getPrimary()->Get_stop_in_detector() << "\n";
//         event->getSimulatedEvent()->getPrimary()->print();

        int sec_num = event->getSimulatedEvent()->getSecondaries().size();

        int pim_mult = 0;
        int pip_mult = 0;
        int piz_mult = 0;
        int pim_mult_1stgen = 0;
        int pip_mult_1stgen = 0;
        int piz_mult_1stgen = 0;

        if (sec_num)
        {
//             std::cout << "Number of secs: "<< event->getSimulatedEvent()->getSecondaries().size() << "\n\n";
            secs += event->getSimulatedEvent()->getSecondaries().size();

            for (int i = 0; i < sec_num; ++i)
            {
                B1Particle * s = event->getSimulatedEvent()->getSecondaries()[i];

                if (s->isPim())
                    ++pim_mult;
                else if (s->isPip())
                    ++pip_mult;
                else if (s->isPiz())
                    ++piz_mult;

                if (s->getParentID() == 1)
                {
                    if (s->isPim())
                        ++pim_mult_1stgen;
                    else if (s->isPip())
                        ++pip_mult_1stgen;
                    else if (s->isPiz())
                        ++piz_mult_1stgen;
                }
            }

            h_pi_mult->Fill(pim_mult + pim_mult + piz_mult);
            h_pim_mult->Fill(pim_mult);
            h_pip_mult->Fill(pip_mult);
            h_piz_mult->Fill(piz_mult);

            h_pi_mult_1stgen->Fill(pim_mult_1stgen + pim_mult_1stgen + piz_mult_1stgen);
            h_pim_mult_1stgen->Fill(pim_mult_1stgen);
            h_pip_mult_1stgen->Fill(pip_mult_1stgen);
            h_piz_mult_1stgen->Fill(piz_mult_1stgen);
        }

        if (event->getSimulatedEvent()->getPrimary()->getStopInDetector())
            dataManager.fill();
    }

//     dataManager.save();

    std::cout << "Number of secs total: " << secs << "\n";

    h_pi_mult->Write();
    h_pim_mult->Write();
    h_pip_mult->Write();
    h_piz_mult->Write();

    h_pi_mult_1stgen->Write();
    h_pim_mult_1stgen->Write();
    h_pip_mult_1stgen->Write();
    h_piz_mult_1stgen->Write();

    dataManager.save();
}