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
    var waypointCount: Int = 0
    var coachingOverlay: ARCoachingOverlayView!
    var selectedEntity: Entity?
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
        guard let selected = entities.first else {
            // Nothing was selected, so add to the view
            addWaypoint(at: touchInView)
            return
        }
        
        if let entity = selectedEntity {
            if selected == entity {
                selectedEntity = nil
            } else {
                let direction = entity.position(relativeTo: selected)
                print("Distance is \(direction.lengthHorizontal) meters in the direction \(direction)")
            }
        } else {
            selectedEntity = selected
        }
    }

    func addWaypoint(at point: CGPoint) {
        guard let raycastQuery = self.makeRaycastQuery(from: point, allowing: .existingPlaneInfinite, alignment: .horizontal) else {
            print("failed first")
            return
        }

        guard let result = self.session.raycast(raycastQuery).first else {
            print("failed")
            return
        }

        var transformation = Transform(matrix: result.worldTransform)
        print("Waypoint Transform")
        print(transformation.matrix)
        transformation.translation += [0, 0.1, 0]
        let box = Waypoint(color: .systemBlue, number: waypointCount)
        self.installGestures(for: box)
        box.generateCollisionShapes(recursive: true)
        box.transform = transformation
        box.orientation = simd_quatf(angle: .pi/4, axis: [0, 0, 1])

        let raycastAnchor = AnchorEntity(raycastResult: result)
        raycastAnchor.addChild(box)
        self.scene.addAnchor(raycastAnchor)
        print(raycastAnchor.transform.matrix)
        waypointCount += 1
        
        waypoints.insert(box)
    }
    
    func addRobot(anchor: ARImageAnchor){
        
        var transformation = Transform(matrix: anchor.transform)
        transformation.translation += [0, 0.02, 0]
        let roboBox = RoboWaypoint(color: .systemRed)
        
        roboBox.transform = transformation
        roboBox.orientation = simd_quatf(angle: .pi/4, axis: [0, 0, 1])

        let robotEntity = AnchorEntity(anchor: anchor)
        robotEntity.addChild(roboBox)
        self.scene.addAnchor(robotEntity)
        self.robot = roboBox
        print("Anchor Added")
    }
    
    func updateRobot(anchor: ARImageAnchor){
        
        var transformation = Transform(matrix: anchor.transform)
        transformation.translation += [0, 0.02, 0]
        
        if(robot == nil){
            self.addRobot(anchor: anchor)
            return
        }
        
        robot!.transform = transformation
        robot!.orientation = simd_quatf(angle: .pi/4, axis: [0, 0, 1])
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
