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

class WaypointView: ARView {
    var coachingOverlay: ARCoachingOverlayView!
    
    var currentWayPoint: Waypoint?
    var robot: RoboWaypoint?
    var lastKnownLocation: RoboWaypoint?
    var lastCheckpoint: Checkpoint?
    
    var waypoints: WaypointList = WaypointList()

    func setupGestures() {
        let tap = UITapGestureRecognizer(target: self, action: #selector(self.handleTap(_:)))
        self.addGestureRecognizer(tap)
    }
    
    @objc func handleTap(_ sender: UITapGestureRecognizer? = nil) {
        guard let touchInView = sender?.location(in: self) else {
            return
        }
        
        // To find whether an entity exists at the point of contact
        let entities = self.entities(at: touchInView)
        guard let _ = entities.first else {
            // Nothing was selected, so add to the view
            addWaypoint(at: touchInView)
            return
        }
    }

    func addWaypoint(at point: CGPoint) {
        guard let raycastQuery = self.makeRaycastQuery(from: point, allowing: .existingPlaneInfinite, alignment: .horizontal) else {
            print("Making raycast query failed")
            return
        }

        guard let result = self.session.raycast(raycastQuery).first else {
            print("Raycast failed")
            return
        }
        
        var transformation = Transform(matrix: result.worldTransform)
        transformation.translation += [0, 0.1, 0]
        
        let box = Waypoint(color: .systemBlue, number: waypoints.count)
        self.installGestures(for: box)
        box.generateCollisionShapes(recursive: true)
        box.transform = transformation
        box.orientation = simd_quatf(angle: .pi/4, axis: [0, 0, 1])

        let raycastAnchor = AnchorEntity(raycastResult: result)
        raycastAnchor.addChild(box)
        self.scene.addAnchor(raycastAnchor)
        waypoints.insert(box)
        
        if(currentWayPoint == nil){
            currentWayPoint = box;
        }
    }
    
    func addRobot(anchor: ARImageAnchor) {
        if let anchor = lastKnownLocation?.parent as? AnchorEntity {
            self.scene.removeAnchor(anchor)
            self.lastKnownLocation = nil
        }
        
        var transformation = Transform(matrix: anchor.transform)
        transformation.translation += [0, 0.02, 0]
        let roboBox = RoboWaypoint(color: .systemRed)
        
        roboBox.transform = transformation
        roboBox.setOrientation(simd_quatf(angle: .pi/4, axis: [0, 0, 1]), relativeTo: roboBox)
        
        let robotEntity = AnchorEntity(anchor: anchor)
        robotEntity.addChild(roboBox)
        
        self.scene.addAnchor(robotEntity)
        self.robot = roboBox
    
        self.lastCheckpoint = Checkpoint(reference: roboBox, orientation: 0.0)
    }
    
    func updateRobot(anchor: ARImageAnchor){
        
        if !anchor.isTracked {
            self.robot?.isEnabled = false
            self.lastKnownLocation = self.robot
            
            self.robot = nil
            print("Lost track of robot")
            return
        }
        
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
