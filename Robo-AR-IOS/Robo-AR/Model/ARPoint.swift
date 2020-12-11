//
//  ARPoint.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/28/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit
import RealityKit

class ARPoint: Entity, HasModel, HasAnchoring, HasCollision {
    var text:String
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
    
    func distanceTo(_ other:ARPoint) -> Float {
        return self.parent!.position(relativeTo: other.parent!).lengthHorizontal
    }
    
    func angleTo(_ other: ARPoint) -> Float {
        var diff = other.parent!.position(relativeTo: self.parent!)
        print("vector between two points is \(diff)")
        diff.y = 0
        return forwardAngle.horizontalAngle(to: diff)
    }
    
    func distanceFromOrigin() -> Float {
        return self.parent!.position.lengthHorizontal
    }
    
    func angleFromOrigin() -> Float {
        var diff = self.parent!.position
        print("vector between two points is \(diff)")
        diff.y = 0
        return forwardAngle.horizontalAngle(to: diff)
    }
    
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
