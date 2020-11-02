//
//  ViewController.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/2/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit
import RealityKit
import ARKit

class ViewController: UIViewController {
    
    @IBOutlet var arView: ARView!
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // arView.addCoaching()
            
        // Load the "Box" scene from the "Experience" Reality File
//         let boxAnchor = try! Experience.loadBox()
        
        // Add the box anchor to the scene
//         arView.scene.anchors.append(boxAnchor)

        let box = Waypoint(color: .yellow)
        arView.installGestures(for: box)
        box.generateCollisionShapes(recursive: true)
        arView.scene.anchors.append(box)

    }
}

extension ARView {
  func setupGestures() {
        let tap = UITapGestureRecognizer(target: self, action: #selector(self.handleTap(_:)))
        self.addGestureRecognizer(tap)
    }
    
    @objc func handleTap(_ sender: UITapGestureRecognizer? = nil) {
        
        guard let touchInView = sender?.location(in: self) else {
            return
        }
        
        rayCastingMethod(point: touchInView)
        
        //to find whether an entity exists at the point of contact
        let entities = self.entities(at: touchInView)
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
            "Todo",
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
    }
}

extension ARView: ARCoachingOverlayViewDelegate {
    func addCoaching() {
        
        let coachingOverlay = ARCoachingOverlayView()
        coachingOverlay.delegate = self
        coachingOverlay.session = self.session
        coachingOverlay.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        
        coachingOverlay.goal = .anyPlane
        self.addSubview(coachingOverlay)
    }
    
    public func coachingOverlayViewDidDeactivate(_ coachingOverlayView: ARCoachingOverlayView) {
        //Ready to add entities next?
    }
}
