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
                print("Distance is \(direction.magnitude) meters in the direction \(direction)")
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

        let transformation = Transform(matrix: result.worldTransform)
        let box = Waypoint(color: .yellow, number: waypointCount)
        self.installGestures(for: box)
        box.generateCollisionShapes(recursive: true)
        box.transform = transformation

        

        let raycastAnchor = AnchorEntity(raycastResult: result)
        raycastAnchor.addChild(box)
        self.scene.addAnchor(raycastAnchor)

        waypointCount += 1
        
        waypoints.insert(box)
    }
}

extension SIMD3 where Scalar == Float {
    var magnitude: Float {
        return sqrtf(x * x + y * y + z * z)
    }
}
