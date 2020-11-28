
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
        
        var diff = other.parent!.position(relativeTo: self.parent!)
        diff.y = 0
        let forwardAngle = SIMD3<Float>(0.0, 0.0, 1.0)
        let angle = acos(dot(forwardAngle, diff) / length(diff)) * 180 / .pi
        
        print(forwardAngle.horizontalAngle(to: diff))
        print(angle)
        return angle
    }
    
    private func constructText() {
        let mesh = MeshResource.generateText(
            "ROBOT",
            extrusionDepth: 0.1,
            font: .systemFont(ofSize: 2),
            containerFrame: .zero,
            alignment: .center,
            lineBreakMode: .byTruncatingTail)
        
        let mesh2 = MeshResource.generateSphere(radius: 1)
        
        let material = SimpleMaterial(color: .systemBlue, isMetallic: false)

        let entity = ModelEntity(mesh: mesh, materials: [material])
        let entity2 = ModelEntity(mesh: mesh2, materials: [material])
        
        entity.scale = SIMD3<Float>(0.01, 0.01, 0.03)
        entity.setOrientation(simd_quatf(angle: -.pi/4, axis: [0, 0, 1]), relativeTo: self)
        entity.setPosition(SIMD3<Float>(0, 0.05, 0), relativeTo: self)
        
        entity2.scale = SIMD3<Float>(0.01, 0.01, 0.01)
        entity2.setPosition(SIMD3<Float>(0, 0, 0.03), relativeTo: self)

        self.addChild(entity)
        self.addChild(entity2)
    }
    
    func describeDestination(distance: Float, angle: Float) {
        
        let mesh = MeshResource.generateSphere(radius: 1)
        let material = SimpleMaterial(color: .systemGreen, isMetallic: false)
        let entity = ModelEntity(mesh: mesh, materials: [material])
        entity.scale = SIMD3<Float>(0.01, 0.01, 0.01)
        entity.setOrientation(simd_quatf(angle: angle, axis: [0, 1, 0]), relativeTo: self)
        entity.setPosition(SIMD3<Float>(0, 0, distance), relativeTo: self)
        self.addChild(entity)
    }
}
