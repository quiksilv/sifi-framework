// @(#)lib/fibers_stack:$Id$
// Author: Rafal Lalik  18/11/2017

/*************************************************************************
 * Copyright (C) 2017-2018, Rafał Lalik.                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $SiFiSYS/LICENSE.                         *
 * For the list of contributors see $SiFiSYS/README/CREDITS.             *
 *************************************************************************/

#include "SFibersStackClusterFinder.h"
#include "SCategory.h"
#include "SFibersStackCluster.h"
#include "SFibersStackClusterFinderPar.h"
#include "SFibersStackGeomPar.h"
#include "SFibersStackHit.h"
#include "SLocator.h"
#include "SLookup.h"
#include "SiFi.h"

#include <RtypesCore.h>

#include <cstdio>
#include <iostream>
#include <math.h>

/**
 * \class SFibersStackClusterFinder
\ingroup lib_fibers_stack

A hit finder task for Fibers Stack.

Takes each fiber data and tries to reconstruct hit along the fiber.

*/

/**
 * Test whether the two hits overlaps withing the errors and tolerance.
 * \param a first hits
 * \param b second hit
 * \param tolerance
 * \return overalp
 */
bool check_overlap(SFibersStackHit* a, SFibersStackHit* b, Float_t tolerance)
{
    // calculate difference and make it sositive
    TVector3 dpos = a->getPoint() - b->getPoint();
    dpos.SetXYZ(fabs(dpos.X()), fabs(dpos.Y()), fabs(dpos.Z()));

    // maximal overlap range with tolerance included
    TVector3 dsigma = a->getErrors() + b->getErrors() + TVector3(tolerance, tolerance, tolerance);

    // compare each dimensions, value > 0 means no overlap
    dpos -= dsigma;

    if (dpos.X() > 0) return false;
    if (dpos.Y() > 0) return false;
    if (dpos.Z() > 0) return false;
    return true;
}

/**
 * Init task
 *
 * \sa STask::init()
 * \return success
 */
bool SFibersStackClusterFinder::init()
{

    // get Cal category
    catFibersHit = sifi()->getCategory(SCategory::CatFibersStackHit);
    if (!catFibersHit)
    {
        std::cerr << "No CatFibersStackHit category" << std::endl;
        return false;
    }

    // create Cluster category
    catFibersCluster = sifi()->buildCategory(SCategory::CatFibersStackClus);
    if (!catFibersCluster)
    {
        std::cerr << "No CatFibersStackCluster category" << std::endl;
        return false;
    }

    // get cluster finder parameters
    pClusterFinderPar = dynamic_cast<SFibersStackClusterFinderPar*>(
        pm()->getParameterContainer("SFibersStackClusterFinderPar"));
    if (!pClusterFinderPar)
    {
        std::cerr << "Parameter container 'SFibersStackClusterFinderPar' was not obtained!"
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    pGeomPar =
        dynamic_cast<SFibersStackGeomPar*>(pm()->getParameterContainer("SFibersStackGeomPar"));
    if (!pGeomPar)
    {
        std::cerr << "Parameter container 'SFibersStackGeomPar' was not obtained!" << std::endl;
        exit(EXIT_FAILURE);
    }

    return true;
}

/**
 * Execute task
 *
 * \sa STask::execute()
 * \return success
 */
bool SFibersStackClusterFinder::execute()
{
    std::vector<std::vector<double>> points;

    const size_t max_mod = pGeomPar->getModules();

    struct Cluster
    {
        int id{-1};
        TVector3 pos;
        TVector3 err;
        Float_t e{0.0};
        std::vector<SFibersStackHit*> hits;
    };
    std::vector<Cluster> clusters;
    std::map<SFibersStackHit*, int>
        hit_cluster_map; // inr = -1: no cluster assigned, id >= cluster id;

    int size = catFibersHit->getEntries();
    for (int i = 0; i < size; ++i)
    {
        SFibersStackHit* pHit = dynamic_cast<SFibersStackHit*>(catFibersHit->getObject(i));
        if (!pHit)
        {
            printf("FibersStackHit doesn't exists!\n");
            continue;
        }
        Int_t mod = 0;
        Int_t lay = 0;
        Int_t fib = 0;
        pHit->getAddress(mod, lay, fib);

        hit_cluster_map[pHit] = -1;
        //         m_points[mod].push_back(MeanShift::Point{pHit->getPoint().X(),
        //         pHit->getPoint().Y(), pHit->getPoint().Z()}); pHit->print();
    }

    int cluster_id = -1;
    int unassigned = hit_cluster_map.size();
    const float tolerance = 0.1;

    //     printf(" => clustering\n");
    for (auto h = hit_cluster_map.begin(); h != hit_cluster_map.end(); ++h)
    {
        // if hit doesn't belong to any cluter, create one

        if (h->second == -1)
        {
            h->second = ++cluster_id;

            // creating new cluster
            Cluster clus;
            clus.id = cluster_id;
            clus.hits.push_back(h->first);
            clusters.push_back(clus);

            --unassigned;
        }
        else
            continue;

        // Loop over other hits trying to match them, repeat a many times as not assigned hits yet.
        // First go over other hits, and try to mach them with any hit in the cluster. If overlap
        // found, assign hit to cluster, and continue
        for (int i = 0; i < unassigned; ++i)
        {
            for (auto hit_nr = 0; hit_nr < clusters[cluster_id].hits.size(); ++hit_nr)
            {
                for (auto h2 = h; h2 != hit_cluster_map.end(); ++h2)
                {

                    if (h2->second != -1) continue; // skip assigned hits

                    if (check_overlap(clusters[cluster_id].hits[hit_nr], h2->first, tolerance))
                    {
                        // overlap found
                        h2->second = cluster_id; // assign cluster to hit
                        clusters[cluster_id].hits.push_back(h2->first);
                        --unassigned;
                    }
                }
            }
        }
    }

    //     printf(" => hits assigned\n");
    //     for (auto h : hit_cluster_map) {
    //         printf("  %#lx -> %d\n", h.first, h.second);
    //     }

    //     printf(" => clusters found\n");

    int mode = pClusterFinderPar->getClusterMode();
    int clus_cnt[max_mod];
    for (int i = 0; i < max_mod; ++i)
        clus_cnt[i] = 0;

    for (int i = 0; i < clusters.size(); ++i)
    {
        float hf_e = 0.; // temp variable for HF mode highest energy
        float ff_z = 0.; // temp variable for FF for minimal Z
        TVector3 weight_sum;

        for (auto h : clusters[i].hits)
        {
            //             h->print();

            if (mode == 0)
            { // AC
                TVector3 e = h->getErrors();
                TVector3 np = h->getPoint();
                TVector3 weight(h->getE() / e.X() / e.X(), h->getE() / e.Y() / e.Y(),
                                h->getE() / e.Z() / e.Z());

                np.SetX(np.X() * weight.X());
                np.SetY(np.Y() * weight.Y());
                np.SetZ(np.Z() * weight.Z());

                clusters[i].pos += np;
                clusters[i].err += weight;
                weight_sum += weight;
            }
            else if (mode == 1)
            { // HF
                if (h->getE() > hf_e)
                {
                    TVector3 e = h->getErrors();
                    TVector3 np = h->getPoint();
                    TVector3 weight(h->getE() / e.X() / e.X(), h->getE() / e.Y() / e.Y(),
                                    h->getE() / e.Z() / e.Z());

                    np.SetX(np.X() * weight.X());
                    np.SetY(np.Y() * weight.Y());
                    np.SetZ(np.Z() * weight.Z());

                    clusters[i].pos = np;
                    clusters[i].err = weight;
                    weight_sum += weight;
                    hf_e = h->getE();
                }
            }
            else if (mode == 2)
            { // FF
                if (ff_z == 0. or h->getPoint().Z() < ff_z)
                {
                    TVector3 e = h->getErrors();
                    TVector3 np = h->getPoint();
                    TVector3 weight(h->getE() / e.X() / e.X(), h->getE() / e.Y() / e.Y(),
                                    h->getE() / e.Z() / e.Z());

                    np.SetX(np.X() * weight.X());
                    np.SetY(np.Y() * weight.Y());
                    np.SetZ(np.Z() * weight.Z());

                    clusters[i].pos = np;
                    clusters[i].err = weight;
                    weight_sum = weight;
                    ff_z = h->getPoint().Z();
                }
                else if (h->getPoint().Z() == clusters[i].pos.Z())
                {
                    TVector3 e = h->getErrors();
                    TVector3 np = h->getPoint();
                    TVector3 weight(h->getE() / e.X() / e.X(), h->getE() / e.Y() / e.Y(),
                                    h->getE() / e.Z() / e.Z());

                    np.SetX(np.X() * weight.X());
                    np.SetY(np.Y() * weight.Y());
                    np.SetZ(np.Z() * weight.Z());

                    clusters[i].pos += np;
                    clusters[i].err += weight;
                    weight_sum += weight;
                }
            }
            else
            {
                printf("Wrong clustering mode!\n");
            }

            clusters[i].e += h->getE();
        }

        clusters[i].pos.SetX(clusters[i].pos.X() / weight_sum.X());
        clusters[i].pos.SetY(clusters[i].pos.Y() / weight_sum.Y());
        clusters[i].pos.SetZ(clusters[i].pos.Z() / weight_sum.Z());

        clusters[i].err.SetX(1. / sqrt(clusters[i].err.X()));
        clusters[i].err.SetY(1. / sqrt(clusters[i].err.Y()));
        clusters[i].err.SetZ(1. / sqrt(clusters[i].err.Z()));

        int m, l, f;
        clusters[i].hits[0]->getAddress(m, l, f);

        SLocator loc(2);
        loc[0] = m;
        loc[1] = clus_cnt[m]++;

        SFibersStackCluster* pCluster =
            reinterpret_cast<SFibersStackCluster*>(catFibersCluster->getSlot(loc));
        if (pCluster) pCluster = new (pCluster) SFibersStackCluster;

        if (pCluster)
        {
            pCluster->setAddress(loc[0], loc[1]);
            pCluster->getPoint() = clusters[i].pos;
            pCluster->getErrors() = clusters[i].err;
        }
        else
        {
            printf("Cluster of m=%ld with id=%ld could not be add.\n", loc[0], loc[1]);
        }
    }

    return true;
}

/**
 * Finalize task
 *
 * \sa STask::finalize()
 * \return success
 */
bool SFibersStackClusterFinder::finalize() { return true; }
