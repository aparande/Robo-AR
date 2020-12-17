//
//  WaypointView+Coaching.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/6/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit
import ARKit
import RealityKit

/**
 Extension to handle calibration of the AR View
 Most of these are system functions that are automatically called.
 Reference: https://heartbeat.fritz.ai/introduction-to-realitykit-on-ios-entities-gestures-and-ray-casting-8f6633c11877
 */
extension WaypointView: ARCoachingOverlayViewDelegate {
    func addCoaching() {
        
        // Initialize the coaching view
        coachingOverlay = ARCoachingOverlayView()
        coachingOverlay.delegate = self
        coachingOverlay.session = self.session
        
        // Set the instructions to find a horizontal plane
        coachingOverlay.goal = .horizontalPlane
        
        // Add the coaching view to the screen
        self.addSubview(coachingOverlay)
        NSLayoutConstraint.activate([
            NSLayoutConstraint(item: self.coachingOverlay!, attribute: .top, relatedBy: .equal, toItem: self, attribute: .top, multiplier: 1.0, constant: 0),
            NSLayoutConstraint(item: self.coachingOverlay!, attribute: .bottom, relatedBy: .equal, toItem: self, attribute: .bottom, multiplier: 1.0, constant: 0),
            NSLayoutConstraint(item: self.coachingOverlay!, attribute: .leading, relatedBy: .equal, toItem: self, attribute: .leading, multiplier: 1.0, constant: 0),
            NSLayoutConstraint(item: self.coachingOverlay!, attribute: .trailing, relatedBy: .equal, toItem: self, attribute: .trailing, multiplier: 1.0, constant: 0)
        ])
        
        coachingOverlay.translatesAutoresizingMaskIntoConstraints = false
        coachingOverlay.activatesAutomatically = true
    }
    
    public func coachingOverlayViewWillActivate(_ coachingOverlayView: ARCoachingOverlayView) {
        print("About to activate overlay")
    }
    
    public func coachingOverlayViewDidDeactivate(_ coachingOverlayView: ARCoachingOverlayView) {
        print("Deactivating Coaching Overlay")
    }
    
    public func coachingOverlayViewDidRequestSessionReset(_ coachingOverlayView: ARCoachingOverlayView) {
        print("Stuff broke so the overlay needs a reset")
    }
}
