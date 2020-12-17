//
//  ARPoint.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/28/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit
import RealityKit

/**
 An abstract class which represents a point in AR Space. Can either be a Waypoint or the Robot.
 Loosely based on https://heartbeat.fritz.ai/introduction-to-realitykit-on-ios-entities-gestures-and-ray-casting-8f6633c11877
 */
class ARPoint: Entity, HasModel, HasAnchoring, HasCollision {
    // Text to display on robot
    var text:String
    
    // Which way is forward
    var forwardAngle = SIMD3<Float>(0.0, 0.0, -1.0)
    
    init(color: UIColor, text: String) {
        self.text = text
        super.init()
        
        self.components[ModelComponent] = ModelComponent(
            mesh: .generateBox(size: SIMD3<Float>(0.05, 0.05, 0.025)),
            materials: [SimpleMaterial(
                color: color,
                isMetallic: false)
            ]
        )
        
        constructText()
    }
    
    required init() {
        fatalError("init() has not been implemented")
    }
    
    /**
    Compute the xz distance to another AR Point using the parent anchor positions
     */
    func distanceTo(_ other:ARPoint) -> Float {
        return self.parent!.position(relativeTo: other.parent!).lengthHorizontal
    }
    
    /**
    Compute the xz angle to another AR Point using parent anchor positions
     */
    func angleTo(_ other: ARPoint) -> Float {
        // Get a vector from the source to destination position
        var diff = other.parent!.position(relativeTo: self.parent!)
        print("vector between two points is \(diff)")
        // Zero out the y-component to make sure we get the xz angle
        diff.y = 0
        
        // Compute the angle between what is forward and the vector from source to destination
        return forwardAngle.horizontalAngle(to: diff)
    }
    
    /**
    How far is the AR point from the origin
     */
    func distanceFromOrigin() -> Float {
        return self.parent!.position.lengthHorizontal
    }
    
    /**
    What is the angle of the vector from the origin to the AR Point
     */
    func angleFromOrigin() -> Float {
        var diff = self.parent!.position
        print("vector between two points is \(diff)")
        diff.y = 0
        return forwardAngle.horizontalAngle(to: diff)
    }
    
    /**
     Helper function to construct the text mesh the user sees
     */
    private func constructText() {
        let mesh = MeshResource.generateText(
            self.text,
            extrusionDepth: 0.1,
            font: .systemFont(ofSize: 2),
            containerFrame: .zero,
            alignment: .center,
            lineBreakMode: .byTruncatingTail)
                
        let material = SimpleMaterial(color: .systemBlue, isMetallic: false)

        let entity = ModelEntity(mesh: mesh, materials: [material])
        
        entity.scale = SIMD3<Float>(0.01, 0.01, 0.03)
        entity.setOrientation(simd_quatf(angle: -.pi/4, axis: [0, 0, 1]), relativeTo: self)
        entity.setPosition(SIMD3<Float>(0, 0.05, 0), relativeTo: self)
        
        self.addChild(entity)
    }
}
