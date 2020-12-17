//
//  WaypointView.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/6/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit
import ARKit
import RealityKit

/**
 The AR View that is the main screen of the application.
 This is the view element which users can tap to place waypoints and see where they are in AR
 It also localizes the robot.
 */
class WaypointView: ARView {
    // The instruction view which tells the user how to calibrate AR
    var coachingOverlay: ARCoachingOverlayView!
    
    // Current waypoint
    var currentWayPoint: Waypoint?
    
    // Robot Location
    var robot: RoboWaypoint?
    
    // Saves the AR Anchor of the last known robot location
    var lastKnownLocation: RoboWaypoint?
    
    // The Last Checkpoint
    var lastCheckpoint: Checkpoint?
    
    // Flags whether the world origin is the Robot
    var didSetOrigin: Bool = false
    
    var waypoints: WaypointList = WaypointList()

    /**
     Register the tap gesture that allows the user to add waypoints by tapping the screen
     */
    func setupGestures() {
        let tap = UITapGestureRecognizer(target: self, action: #selector(self.handleTap(_:)))
        self.addGestureRecognizer(tap)
    }
    
    /**
     Function triggered when user taps the screen
     */
    @objc func handleTap(_ sender: UITapGestureRecognizer? = nil) {
        if !didSetOrigin {
            print("Locate robot first!")
            return
        }
        
        guard let touchInView = sender?.location(in: self) else {
            return
        }
        
        // To find whether an entity exists at the point of contact
        let entities = self.entities(at: touchInView)
        
        guard let _ = entities.first else {
            // No entity exists at the touch, so add a new waypoint
            addWaypoint(at: touchInView)
            return
        }
    }

    /**
     Add a new waypoint to a point in the screen
     */
    func addWaypoint(at point: CGPoint) {
        // Raycast from the screen coordinates to world coordinates
        guard let raycastQuery = self.makeRaycastQuery(from: point, allowing: .existingPlaneInfinite, alignment: .horizontal) else {
            print("Making raycast query failed")
            return
        }

        guard let result = self.session.raycast(raycastQuery).first else {
            print("Raycast failed")
            return
        }
        
        // Compute the world position of the tap
        var transformation = Transform(matrix: result.worldTransform)
        transformation.translation += [0, 0.1, 0]
        
        // Build a waypoint
        let box = Waypoint(color: .systemBlue, number: waypoints.count)
        self.installGestures(for: box)
        box.generateCollisionShapes(recursive: true)
        box.transform = transformation
        box.orientation = simd_quatf(angle: .pi/4, axis: [0, 0, 1])

        // Assign the waypoint to an anchor placed at the raycast result
        let raycastAnchor = AnchorEntity(raycastResult: result)
        raycastAnchor.addChild(box)
        self.scene.addAnchor(raycastAnchor)
        
        // Save the waypoint in the list
        waypoints.insert(box)
        
        if(currentWayPoint == nil){
            currentWayPoint = box;
        }
    }
    
    /**
     Attach the robot marker where the AR Tag Image Anchor is
     */
    func addRobot(anchor: ARImageAnchor) {
        // Make sure to remove any old robot waypoints from memory
        if let anchor = lastKnownLocation?.parent as? AnchorEntity {
            self.scene.removeAnchor(anchor)
            self.lastKnownLocation = nil
        }
        
        // Build the robot waypoint
        var transformation = Transform(matrix: anchor.transform)
        transformation.translation += [0, 0.02, 0]
        let roboBox = RoboWaypoint(color: .systemRed)

        roboBox.transform = transformation
        roboBox.setOrientation(simd_quatf(angle: .pi/4, axis: [0, 0, 1]), relativeTo: roboBox)
        
        let robotEntity = AnchorEntity(anchor: anchor)
        robotEntity.addChild(roboBox)
        
        self.scene.addAnchor(robotEntity)
        self.robot = roboBox
        
        // Set the world world origin to the first time the robot is localized
        if !didSetOrigin {
            self.session.setWorldOrigin(relativeTransform: anchor.transform)
            didSetOrigin = true
        }
    
        // Create a checkpoint for later in case the robot goes off screen
        self.lastCheckpoint = Checkpoint(reference: roboBox, orientation: 0.0)
    }
    
    /**
     Update the location of the robot waypoint when the aruco marker moves
     */
    func updateRobot(anchor: ARImageAnchor){
        // If we lose track of the robot, then save its last known location
        if !anchor.isTracked {
            self.robot?.isEnabled = false
            self.lastKnownLocation = self.robot
            
            self.robot = nil
            print("Lost track of robot")
            return
        }
        
        // Update the robot waypoint's location
        var transformation = Transform(matrix: anchor.transform)
        transformation.translation += [0, 0.02, 0]
        if let robot = self.robot {
            robot.transform = transformation
            robot.setOrientation(simd_quatf(angle: .pi/4, axis: [0, 0, 1]), relativeTo: robot)
        } else {
            self.addRobot(anchor: anchor)
        }
    }
}
