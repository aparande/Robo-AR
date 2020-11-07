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

    /*
    // Only override draw() if you perform custom drawing.
    // An empty implementation adversely affects performance during animation.
    override func draw(_ rect: CGRect) {
        // Drawing code
    }
    */
    
    func setupGestures() {
      let tap = UITapGestureRecognizer(target: self, action: #selector(self.handleTap(_:)))
      self.addGestureRecognizer(tap)
    }
    
    @objc func handleTap(_ sender: UITapGestureRecognizer? = nil) {
          guard let touchInView = sender?.location(in: self) else {
              return
          }
          
          rayCastingMethod(point: touchInView)
          
          // To find whether an entity exists at the point of contact
          // let entities = self.entities(at: touchInView)
    }
      
    func rayCastingMethod(point: CGPoint) {
        guard let raycastQuery = self.makeRaycastQuery(from: point, allowing: .existingPlaneInfinite, alignment: .horizontal) else {
              print("failed first")
              return
          }
          
          guard let result = self.session.raycast(raycastQuery).first else {
              print("failed")
              return
          }
          
          let transformation = Transform(matrix: result.worldTransform)
          let box = Waypoint(color: .yellow)
          self.installGestures(for: box)
          box.generateCollisionShapes(recursive: true)
          
          let mesh = MeshResource.generateText(
              "\(waypointCount)",
              extrusionDepth: 0.1,
              font: .systemFont(ofSize: 2),
              containerFrame: .zero,
              alignment: .left,
              lineBreakMode: .byTruncatingTail)

          let material = SimpleMaterial(color: .red, isMetallic: false)
          let entity = ModelEntity(mesh: mesh, materials: [material])
          entity.scale = SIMD3<Float>(0.03, 0.03, 0.1)

          box.addChild(entity)
          box.transform = transformation

          entity.setPosition(SIMD3<Float>(0, 0.05, 0), relativeTo: box)
          
          let raycastAnchor = AnchorEntity(raycastResult: result)
          raycastAnchor.addChild(box)
          self.scene.addAnchor(raycastAnchor)
        
        waypointCount += 1
    }
}
