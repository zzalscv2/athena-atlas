/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                        FastVertexFitter.cxx  -  Description
 ***************************************************************************/
#include "TrkVertexBilloirTools/FastVertexFitter.h"
#include "VxVertex/VxTrackAtVertex.h"
#include "VxVertex/LinearizedTrack.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/LinkToTrack.h"
#include "TrkParticleBase/LinkToTrackParticleBase.h"
#include "TrkParticleBase/TrackParticleBase.h"
#include "EventPrimitives/EventPrimitives.h"
#include "TrkLinks/LinkToXAODTrackParticle.h" 
//xAOD includes 
#include "xAODTracking/Vertex.h" 
#include "xAODTracking/TrackParticle.h" 

/* These are some local helper classes only needed for convenience, therefor
within anonymous namespace. They do contain temporary calculations of matrices
and vectors resulting from the Billoir calculation (Note: no transformation
of the perigee parameters is done anymore). */
namespace
{
	struct BilloirTrack
	{
		BilloirTrack() : originalPerigee(nullptr),chi2{} { DtWD.setZero(); DtWDx.setZero(); xpVec.setZero();}
		virtual ~BilloirTrack() = default;
		const Trk::TrackParameters * originalPerigee;
		AmgMatrix(3,3) DtWD;
		Amg::Vector3D DtWDx;
		Amg::Vector3D xpVec;
		double chi2;
	};

	struct BilloirVertex
	{
		BilloirVertex() : chi2{},ndf{} { DtWD_Sum.setZero(); DtWDx_Sum.setZero();}
		virtual ~BilloirVertex() = default;
		AmgMatrix(3,3) DtWD_Sum;
		Amg::Vector3D DtWDx_Sum;
		double chi2;
		unsigned int ndf;
	};
}

namespace Trk
{
	StatusCode FastVertexFitter::initialize()
	{

		if ( m_extrapolator.retrieve().isFailure() )
		{
			msg(MSG::FATAL) << "Failed to retrieve tool " << m_extrapolator << endmsg;
			return StatusCode::FAILURE;
		}
		
		
			msg(MSG::INFO) << "Retrieved tool " << m_extrapolator << endmsg;
		

		if ( m_linFactory.retrieve().isFailure() )
		{
			msg(MSG::FATAL) << "Failed to retrieve tool " << m_linFactory << endmsg;
			return StatusCode::FAILURE;
		}
		
		
			msg(MSG::INFO) << "Retrieved tool " << m_linFactory << endmsg;
		

		return StatusCode::SUCCESS;
	}


	FastVertexFitter::FastVertexFitter ( const std::string& t, const std::string& n, const IInterface*  p ) : base_class ( t,n,p ),
														  m_maxIterations ( 3 ),
														  m_maxDchi2PerNdf ( 0.000001 )
														  
														  
	{
	  declareProperty ( "MaxIterations",        m_maxIterations );
	  declareProperty ( "MaxChi2PerNdf",        m_maxDchi2PerNdf );
	  declareInterface<IVertexFitter> ( this );
	}

	FastVertexFitter::~FastVertexFitter() = default;



	/** Interface for Track with vertex constraint */
	/** the position of the constraint is ALWAYS the starting point */
	/** Interface for MeasuredPerigee with starting point */
	xAOD::Vertex * FastVertexFitter::fit ( const std::vector<const Trk::TrackParameters*> & originalPerigees,
	                                       const Amg::Vector3D& firstStartingPoint ) const
	{
                xAOD::Vertex constraint;
                constraint.makePrivateStore();
                constraint.setPosition( firstStartingPoint );
                constraint.setCovariancePosition( AmgSymMatrix(3)::Zero(3,3) );
                constraint.setFitQuality( 0.,0.);
		return fit ( originalPerigees, constraint );
	}

	/** Interface for MeasuredPerigee with vertex constraint */
	/** the position of the constraint is ALWAYS the starting point */
	xAOD::Vertex * FastVertexFitter::fit ( const std::vector<const Trk::TrackParameters*> & originalPerigees,
	                                       const xAOD::Vertex& firstStartingPoint ) const
	{
		if ( originalPerigees.empty() )
		{
			ATH_MSG_VERBOSE("No tracks to fit in this event.");
			return nullptr;
		}

		/* Initialisation of variables */
		double chi2 = 2000000000000.;
		unsigned int nRP = originalPerigees.size();  // Number of tracks to fit
		int ndf = nRP * ( 5-3 ) - 3;                 // Number of degrees of freedom
		if ( ndf == -1 ) ndf = 1;

		/* Determine if we are doing a constraint fit.*/
		bool constraint = false;
		if ( firstStartingPoint.covariancePosition().trace() != 0. )
		{
			constraint = true;
			ndf += 3;
			ATH_MSG_DEBUG("Fitting with constraint.");
			ATH_MSG_VERBOSE(firstStartingPoint.covariancePosition().inverse().eval());
		}

		double chi2New=0.;double chi2Old=0.;

		Amg::Vector3D linPoint ( firstStartingPoint.position() ); // linearization point for track parameters (updated for every iteration)

		auto fittedVertex = std::make_unique<xAOD::Vertex>();
                fittedVertex->makePrivateStore(); // xAOD::VertexContainer will take ownership of AuxStore when ActualVertex is added to it

		std::vector<VxTrackAtVertex> tracksAtVertex;
		std::vector<BilloirTrack> billoirTracks;

		/* Iterate fits until the fit criteria are met, or the number of max
		iterations is reached. */
		for ( unsigned int niter=0; niter < m_maxIterations; ++niter )
		{
			// msg(MSG::VERBOSE) << "Start of iteration " << niter << ", starting point ("
			// << linPoint [0] << ", " << linPoint [1] << ", " << linPoint [2]
			// << ") and " << originalPerigees.size() << " tracks." << endmsg;

			billoirTracks.clear();
			chi2Old = chi2New;
			chi2New = 0.;

			AmgMatrix(2,3) D;

			/* Linearize the track parameters wrt. starting point of the fit */
			Amg::Vector3D globalPosition = linPoint;
			Trk::PerigeeSurface perigeeSurface ( globalPosition );

			BilloirVertex billoirVertex;
                        // unsigned int count(0);
			for (const auto *originalPerigee : originalPerigees)
			{
				LinearizedTrack* linTrack = m_linFactory->linearizedTrack ( originalPerigee, linPoint );
				if ( linTrack==nullptr )
				{
					ATH_MSG_DEBUG("Could not linearize track! Skipping this track!");
				}
				else
				{
					// local position
					Amg::Vector3D locXpVec = linTrack->expectedPositionAtPCA();
					locXpVec[0] = locXpVec[0] - linPoint[0];
					locXpVec[1] = locXpVec[1] - linPoint[1];
					locXpVec[2] = locXpVec[2] - linPoint[2];
					
					// msg(MSG::VERBOSE) << "Track: " << count << endmsg;
					// count++;
					// const Trk::MeasuredPerigee* tmpPerigee = dynamic_cast<const Trk::MeasuredPerigee*>(*iter);
					//AmgVector(5) expParameters = linTrack->expectedParametersAtPCA();
					
					// msg(MSG::VERBOSE) << "locXp: " << locXpVec[0] << "\t" << locXpVec[1] << "\t" << locXpVec[2] << endmsg;

					// first get the cov 2x2 sub matrix and then invert (don't get the 2x2 sub matrix of the 5x5 already inverted cov matrix)
					AmgMatrix(2,2) billoirCovMat = linTrack->expectedCovarianceAtPCA().block<2,2>(0,0);
					// msg(MSG::VERBOSE) << "CovMatrix: " << billoirCovMat[0][0] << "\t" << billoirCovMat[0][1] << endmsg;
					// msg(MSG::VERBOSE) << "           " << billoirCovMat[1][0] << "\t" << billoirCovMat[1][1] << endmsg;
					AmgMatrix(2,2) billoirWeightMat = billoirCovMat.inverse().eval();
					// msg(MSG::VERBOSE) << "WeightMatrix: " << billoirWeightMat[0][0] << "\t" << billoirWeightMat[0][1] << endmsg;
					// msg(MSG::VERBOSE) << "              " << billoirWeightMat[1][0] << "\t" << billoirWeightMat[1][1] << endmsg;
					// D matrix for d0 and z0
					D = linTrack->positionJacobian().block<2,3>(0,0);
					// msg(MSG::VERBOSE) << "DMatrix:      " << D[0][0] << "\t" << D[0][1] << endmsg;
					// msg(MSG::VERBOSE) << "              " << D[1][0] << "\t" << D[1][1] << endmsg;

					// Calculate DtWD and DtWD*x and sum them
					BilloirTrack locBilloirTrack;
					locBilloirTrack.xpVec = locXpVec;
					locBilloirTrack.DtWD = (D.transpose())*billoirWeightMat*D; 
					locBilloirTrack.chi2 = -1.0; 
					billoirVertex.DtWD_Sum += locBilloirTrack.DtWD;
					locBilloirTrack.DtWDx = ((D.transpose())*billoirWeightMat*D)*locXpVec;
					billoirVertex.DtWDx_Sum += locBilloirTrack.DtWDx;
					locBilloirTrack.originalPerigee = originalPerigee;
					billoirTracks.push_back ( locBilloirTrack );
				}
			        delete linTrack; linTrack=nullptr;
			}
			if ( billoirTracks.empty() )
			{
				ATH_MSG_DEBUG("No linearized tracks left after linearization! Should not happen!");
				return nullptr;
			}
			if ( constraint )
			{
				// add V_del += wgtconst * (linPoint.position() - Vconst) and V_wgt +=wgtconst
				Amg::Vector3D constraintPosInBilloirFrame;
				constraintPosInBilloirFrame.setZero();
				// this will be 0 for first iteration but != 0 from second on
				constraintPosInBilloirFrame[0] = firstStartingPoint.position() [0]-linPoint [0];
				constraintPosInBilloirFrame[1] = firstStartingPoint.position() [1]-linPoint [1];
				constraintPosInBilloirFrame[2] = firstStartingPoint.position() [2]-linPoint [2];
				billoirVertex.DtWDx_Sum += firstStartingPoint.covariancePosition().inverse().eval() *constraintPosInBilloirFrame;
				billoirVertex.DtWD_Sum  += firstStartingPoint.covariancePosition().inverse().eval();
			}

			AmgMatrix(3,3) cov_delta_V_mat = billoirVertex.DtWD_Sum.inverse( ) ;

			Amg::Vector3D delta_V = cov_delta_V_mat * billoirVertex.DtWDx_Sum;

			std::vector<BilloirTrack>::iterator BTIter;
			for ( BTIter = billoirTracks.begin(); BTIter != billoirTracks.end() ; ++BTIter )
			{
				// calculate chi2PerTrack = (xpVec - vtxPosInBilloirFrame).T * DtWD(track) * (xpVec - vtxPosInBilloirFrame); )
				Amg::Vector3D diff;
				diff.setZero();
				diff[0] = ( *BTIter ).xpVec[0] - delta_V[0];
				diff[1] = ( *BTIter ).xpVec[1] - delta_V[1];
				diff[2] = ( *BTIter ).xpVec[2] - delta_V[2];
				( *BTIter ).chi2 = diff.dot(( *BTIter ).DtWD* diff );
				if ( ( *BTIter ).chi2 < 0 )
				{
					std::cout << "VxFastFit::calculate: error in chi2_per_track\n";
					return nullptr;
				}
				chi2New += ( *BTIter ).chi2;
			}

			if ( constraint )
			{
				Amg::Vector3D deltaTrk;
				deltaTrk.setZero();
				// last term will also be 0 again but only in the first iteration
				//                  = calc. vtx in billoir frame - (    constraint pos. in billoir frame    )
				deltaTrk[0] = delta_V[0]                 - ( firstStartingPoint.position() [0] - linPoint [0] );
				deltaTrk[1] = delta_V[1]                 - ( firstStartingPoint.position() [1] - linPoint [1] );
				deltaTrk[2] = delta_V[2]                 - ( firstStartingPoint.position() [2] - linPoint [2] );
				double chi2FromConstraint ( ( deltaTrk.transpose() * firstStartingPoint.covariancePosition().inverse().eval() * deltaTrk ) [0] );
				chi2New  += chi2FromConstraint;
			}

			/* assign new linearization point (= new vertex position in global frame) */
			Amg::Vector3D tmpPos ( linPoint );
			tmpPos[0] += delta_V[0];   tmpPos[1] += delta_V[1];   tmpPos[2] += delta_V[2];
			linPoint = tmpPos;

			// msg(MSG::VERBOSE) << "Vertex of Iteration " << niter << " with chi2: " << chi2New << "\t old chi2: " << chi2 << endmsg;
			// msg(MSG::VERBOSE) << "deltaV: ("    << delta_V[0] << ", " << delta_V[1] << ", " << delta_V[2] << ")" << endmsg;
			// msg(MSG::VERBOSE) << linPoint << endmsg;

			if ( chi2New < chi2 )
			{
				/* Store the vertex */
				chi2 = chi2New;
				//const AmgMatrix(3,3) * newCovarianceMatrix = &cov_delta_V_mat ;
				//const AmgMatrix(3,3) newErrorMatrix = newCovarianceMatrix->inverse().eval();
				//fittedVertex = RecVertex ( linPoint.position(), newErrorMatrix, ndf, chi2 );

                                // The cov_delta_V_mat does not need to be inverted.  -katy 2/3/16
				fittedVertex->setPosition( linPoint );
				fittedVertex->setCovariancePosition( cov_delta_V_mat );
				fittedVertex->setFitQuality( chi2, ndf );

				/* new go through vector and delete entries */
				/* // TODO: not needed anymore, tracksAtVertex doesn't store pointers - just the objects themselves <David Shope> (EDM Migration) 03/21/16
				for ( std::vector<Trk::VxTrackAtVertex*>::const_iterator itr = tracksAtVertex.begin();
				        itr != tracksAtVertex.end(); ++itr )
				{
					delete ( *itr );
				}
				*/

				tracksAtVertex.clear();

				Amg::Vector3D pointToExtrapolateTo ( linPoint [0], linPoint [1], linPoint [2] );
				Trk::PerigeeSurface perigeeSurface ( pointToExtrapolateTo );
				for ( BTIter = billoirTracks.begin(); BTIter != billoirTracks.end() ; ++BTIter )
				{
					// you need to extrapolate the original perigee ((*BTIter).originalPerigee) really to the
					// found vertex. The first propagation above is only to the starting point. But here we
					// want to store it wrt. to the last fitted vertex
					auto extrapolatedPerigee = std::unique_ptr<const Trk::TrackParameters> ( m_extrapolator->extrapolate ( 
              Gaudi::Hive::currentContext(), 
              * ( *BTIter ).originalPerigee, 
              perigeeSurface ) );
					if ( extrapolatedPerigee==nullptr )
					{
						extrapolatedPerigee = std::unique_ptr<const Trk::TrackParameters>(((*BTIter).originalPerigee)->clone());
						ATH_MSG_DEBUG("Could not extrapolate these track parameters to final vertex position! Storing original position as final one ...");
					}
          //VxTrackAtVertex will own the clone of the extrapolatedPerigee
					Trk::VxTrackAtVertex* tmpVxTrkAtVtx = new Trk::VxTrackAtVertex ( ( *BTIter ).chi2, extrapolatedPerigee->clone(), 
                                                                           ( *BTIter ).originalPerigee ) ;
					tracksAtVertex.push_back ( *tmpVxTrkAtVtx );
					// TODO: here is where the vxTracksAtVertex pointers are deleted
					delete tmpVxTrkAtVtx; // TODO: is this ok?
				}
			}

			if ( fabs ( chi2Old-chi2New ) < m_maxDchi2PerNdf * ndf )
			{
				break;
			}
		} // end of iteration
		fittedVertex->vxTrackAtVertex() = tracksAtVertex;
		//ATH_MSG_VERBOSE("Final Vertex Fitted: " << fittedVxCandidate->recVertex()); // TODO: can no longer print vertex after converting to xAOD
		return fittedVertex.release();
	}

	xAOD::Vertex * FastVertexFitter::fit ( const std::vector<const Trk::TrackParameters*>& perigeeList ) const
	{
		Amg::Vector3D tmpVtx(0.,0.,0.);
		return fit ( perigeeList, tmpVtx );
	}

  		//xAOD interfaced methods. Required to un-block the current situation  
 		// with the xAOD tracking design. 
 		 xAOD::Vertex * FastVertexFitter::fit(const std::vector<const xAOD::TrackParticle*>& vectorTrk,const Amg::Vector3D& startingPoint)  const
 		 {
                  xAOD::Vertex constraint;
                  constraint.makePrivateStore();
                  constraint.setPosition( startingPoint );
                  constraint.setCovariancePosition( AmgSymMatrix(3)::Zero(3,3) );
                  constraint.setFitQuality( 0.,0.);
 		  return fit(vectorTrk, constraint);                                
 		 }//end of the xAOD starting point fit method 
 		 
 		     
 		 xAOD::Vertex * FastVertexFitter::fit(const std::vector<const xAOD::TrackParticle*>& vectorTrk, const xAOD::Vertex& constraint) const
 		 { 
 		   if(vectorTrk.empty()) 
 		   { 
 		    msg(MSG::INFO)<<"Empty vector of tracks passed"<<endmsg; 
 		    return nullptr; 
 		   } 
 		    
 		   //making a list of perigee out of the vector of tracks   
 		   std::vector<const Trk::TrackParameters*> measuredPerigees; 
 		    
 		   for(const auto *i : vectorTrk) 
 		   { 
 		    const Trk::TrackParameters * tmpMeasPer = &(i->perigeeParameters()); 
 		   
 		    if(tmpMeasPer!=nullptr) measuredPerigees.push_back(tmpMeasPer); 
 		    else  msg(MSG::INFO)<<"Failed to dynamic_cast this track parameters to perigee"<<endmsg; //TODO: Failed to implicit cast the perigee parameters to track parameters?
 		   } 
 		    
 		    
 		   xAOD::Vertex* fittedVertex = fit( measuredPerigees, constraint ); 
 		 
 		   //assigning the input tracks to the fitted vertex through VxTrackAtVertices
 		   { 
 		    if( fittedVertex->vxTrackAtVertexAvailable() ) // TODO: I don't think vxTrackAtVertexAvailable() does the same thing as a null pointer check!
 		    { 
 		     if(!fittedVertex->vxTrackAtVertex().empty()) 
 		     { 
 		      for(unsigned int i = 0; i <vectorTrk.size(); ++i) 
 		      { 
 		 
 		       LinkToXAODTrackParticle * linkTT = new LinkToXAODTrackParticle; 
 		       linkTT->setElement(vectorTrk[i]); 
 		 
 		       // vxtrackatvertex takes ownership! 
 		       ( fittedVertex->vxTrackAtVertex() )[i].setOrigTrack(linkTT); 
 		      }//end of loop for setting orig tracks in. 
 		     }//end of protection against unsuccessfull updates (no tracks were added) 
 		    }//end of vector of tracks check 
 		   }//end of pointer check 

                   //now set links to xAOD::TrackParticles directly in the xAOD::Vertex
                   unsigned int VTAVsize = fittedVertex->vxTrackAtVertex().size();
                   for (unsigned int i = 0 ; i < VTAVsize ; ++i)
                   {
                     Trk::VxTrackAtVertex* VTAV = &( fittedVertex->vxTrackAtVertex().at(i) );
                     //TODO: Will this pointer really hold 0 if no VxTrackAtVertex is found?
                     if (not VTAV){
                       ATH_MSG_WARNING (" Trying to set link to xAOD::TrackParticle. The VxTrackAtVertex is not found");
                       continue;
                     }

                     Trk::ITrackLink* trklink = VTAV->trackOrParticleLink();

                     // See if the trklink is to an xAOD::TrackParticle
                     Trk::LinkToXAODTrackParticle* linkToXAODTP = dynamic_cast<Trk::LinkToXAODTrackParticle*>(trklink);
                     if (linkToXAODTP)
                     {

                       //Now set the new link to the xAOD vertex
                       fittedVertex->addTrackAtVertex(*linkToXAODTP, VTAV->weight());

                     } else {
                       ATH_MSG_WARNING ("Skipping track. Trying to set link to something else than xAOD::TrackParticle. Neutrals not supported.");
                     }
                   } //end of loop

 		   return fittedVertex;

 		 }//end of the xAOD constrained fit method 
  
 		 


}

