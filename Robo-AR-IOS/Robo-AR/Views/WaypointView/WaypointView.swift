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
    
    func addRobot(anchor: ARImageAnchor){
        
        var transformation = Transform(matrix: anchor.transform)
        transformation.translation += [0, 0.02, 0]
        let roboBox = RoboWaypoint(color: .systemRed)
        
        roboBox.transform = transformation
        roboBox.setOrientation(simd_quatf(angle: .pi/4, axis: [0, 0, 1]), relativeTo: roboBox)
        roboBox.isTracking = anchor.isTracked
        let robotEntity = AnchorEntity(anchor: anchor)
        robotEntity.addChild(roboBox)
        
        self.scene.addAnchor(robotEntity)
        self.robot = roboBox
        
    }
    
    func updateRobot(anchor: ARImageAnchor){
        
        if !anchor.isTracked, let anchor = self.robot?.parent as? AnchorEntity {
            self.scene.removeAnchor(anchor)
            self.robot = nil
            print("Lost track of robot")
            return
        }
        
        var transformation = Transform(matrix: anchor.transform)
        transformation.translation += [0, 0.02, 0]
        if let robot = self.robot {
            
            if(robot.isTracking != anchor.isTracked){
                print("TRACKING CHANGE: ",  anchor.isTracked)
            }
            robot.isTracking = anchor.isTracked
            
            robot.transform = transformation
            robot.setOrientation(simd_quatf(angle: .pi/4, axis: [0, 0, 1]), relativeTo: robot)
            if(currentWayPoint != nil){
                robot.lastKnownWaypointAngle = robot.angleTo(currentWayPoint!)
                robot.lastKnownWaypointDistance = robot.distanceTo(currentWayPoint!)
                robot.waypointLastKnownAngleTo = currentWayPoint!.angleTo(robot);
                robot.waypointLastKnownNumber = currentWayPoint!.number
                
            }
        } else {
            self.addRobot(anchor: anchor)
        }
    }
}

extension SIMD3 where Scalar == Float {
    var lengthHorizontal: Float {
        return sqrtf(x * x  + z * z)
    }
    
    func horizontalAngle(to other:SIMD3) -> Float {
        // x axis is oriented such that positive is right
        // z axis is oriented such that positive is towards the user (which is why it is flipped from the angle calculation
        return -atan2f(other.x - self.x, self.z - other.z) * 180 / .pi
    }
}
