
import UIKit
import RealityKit

class RoboWaypoint: Entity, HasModel, HasAnchoring, HasCollision {
    
    required init(color: UIColor) {
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
    
    
    func distanceTo(_ other:Waypoint) -> Float {
        return self.parent!.position(relativeTo: other.parent!).lengthHorizontal
    }
    
    func angleTo(_ other: Waypoint) -> Float {
        return self.parent!.position.horizontalAngle(to: other.parent!.position)
    }
    
    private func constructText() {
        let mesh = MeshResource.generateText(
            "ROBOT",
            extrusionDepth: 0.1,
            font: .systemFont(ofSize: 2),
            containerFrame: .zero,
            alignment: .left,
            lineBreakMode: .byTruncatingTail)

        let material = SimpleMaterial(color: .systemBlue, isMetallic: false)
        let entity = ModelEntity(mesh: mesh, materials: [material])
        entity.scale = SIMD3<Float>(0.01, 0.01, 0.03)
        entity.setPosition(SIMD3<Float>(0.02, 0.04, 0), relativeTo: self)
        entity.setOrientation(simd_quatf(angle: -.pi/4, axis: [0, 0, 1]), relativeTo: self)
        self.addChild(entity)
    }
}
