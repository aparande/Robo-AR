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
import CoreBluetooth


class ViewController: BLEViewController {
    @IBOutlet var arView: WaypointView!
    @IBOutlet weak var generateButton: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        arView.addCoaching()
        arView.setupGestures()
        
        guard let referenceImages = ARReferenceImage.referenceImages(inGroupNamed: "AR Resources", bundle: nil) else {
            fatalError("Missing expected asset catalog resources.")
        }
        
        let config = ARWorldTrackingConfiguration()
        config.detectionImages = referenceImages
        config.maximumNumberOfTrackedImages = 1
        config.planeDetection = .horizontal
        
        arView.session.delegate = self
        arView.session.run(config, options: [.resetTracking, .removeExistingAnchors])
        arView.debugOptions.insert(.showSceneUnderstanding)
        arView.debugOptions.insert(.showWorldOrigin)
        
        generateButton.layer.cornerRadius = 20
    }
    
    @IBAction func startTransmission(_ sender: Any) {
        if let inst = compileNextInstruction(fromAck: false) {
            self.generateButton.isEnabled = false
            sendInstruction(instruction: inst)
        }
    }
    

    override func compileNextInstruction(fromAck: Bool) -> Instruction? {
        if fromAck {
            setNextTarget()
        }
        
        guard let waypoint = arView.currentWayPoint else { return nil }
        
        var inst: Instruction!
        
        // If you have a checkpoint or know the robots location, then use it
        if let trackedObject: TrackedObject = arView.robot ?? arView.lastCheckpoint {
            print("Using last known location from \(type(of: trackedObject))")
            inst = Instruction(distance: trackedObject.distanceTo(waypoint), angle: trackedObject.angleTo(waypoint), waypointNumber: waypoint.number)
            
            // We assume the robot will achieve the checkpoint when if is not being tracked
            arView.lastCheckpoint = Checkpoint(reference: waypoint, orientation: (arView.lastCheckpoint?.orientation ?? 0.0) + inst.angle)
            print("Robot orientation \(arView.lastCheckpoint?.orientation)")
        } else {
            print("Assuming robot at origin")
            // If you have no checkpoints and don't know the robot's location, assume the robot is oriented forward at the origin
            inst = Instruction(distance: waypoint.distanceFromOrigin(), angle: waypoint.angleFromOrigin(), waypointNumber: waypoint.number)
            arView.lastCheckpoint = Checkpoint(reference: waypoint, orientation: inst.angle)
        }
        
        return inst
    }
    
    private func setNextTarget() {
        guard let currentWaypoint = arView.currentWayPoint else { return }
        
        if let robot = arView.robot {
            if (robot.distanceTo(currentWaypoint) < 0.10) {
                print("Accurately hit waypoint")
                arView.currentWayPoint = currentWaypoint.next
                arView.waypoints.head = arView.currentWayPoint
            }
        } else {
            print("Can't see robot. Assuming it was accurate")
            arView.currentWayPoint = currentWaypoint.next
            arView.waypoints.head = arView.currentWayPoint
        }
    }
}

extension ViewController: ARSessionDelegate {
    
    func session(_ session: ARSession, didAdd anchors: [ARAnchor]) {
        guard let imageAnchor = anchors.first as? ARImageAnchor,
              let _ = imageAnchor.referenceImage.name
        else { return }
        arView.addRobot(anchor: imageAnchor)
    }
    
    func session(_ session: ARSession, didUpdate anchors: [ARAnchor]) {
        guard let imageAnchor = anchors.first as? ARImageAnchor,
              let _ = imageAnchor.referenceImage.name
        else { return }
        
        arView.updateRobot(anchor: imageAnchor)
    }
    
    func session(_ session: ARSession, didRemove anchors: [ARAnchor]) {
        guard let imageAnchor = anchors.first as? ARImageAnchor,
              let _ = imageAnchor.referenceImage.name
        else { return }
    }
}
