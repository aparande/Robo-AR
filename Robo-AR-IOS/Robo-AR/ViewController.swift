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

/**
 Main View Controller which takes user interactions from the view and makes decisions about what BLE commands to send
 */
class ViewController: BLEViewController {
    // View references
    @IBOutlet var arView: WaypointView!
    @IBOutlet weak var generateButton: UIButton!
    
    /**
     System method called whenever the view is loaded for the first tim
     */
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Setup the AR View
        arView.addCoaching()
        arView.setupGestures()
        
        // Configure image tracking
        // Based on https://developer.apple.com/documentation/arkit/detecting_images_in_an_ar_experience
        guard let referenceImages = ARReferenceImage.referenceImages(inGroupNamed: "AR Resources", bundle: nil) else {
            fatalError("Missing expected asset catalog resources.")
        }
        let config = ARWorldTrackingConfiguration()
        config.detectionImages = referenceImages
        config.maximumNumberOfTrackedImages = 1
        config.planeDetection = .horizontal
        
        // Configure the AR Session
        arView.session.delegate = self
        arView.session.run(config, options: [.resetTracking, .removeExistingAnchors])
        arView.debugOptions.insert(.showSceneUnderstanding)
        arView.debugOptions.insert(.showWorldOrigin)
        
        // Style the button
        generateButton.layer.cornerRadius = 20
    }
    
    /**
     Function triggered by the user pressing the button
     */
    @IBAction func startTransmission(_ sender: Any) {
        if let inst = compileNextInstruction(fromAck: false) {
            self.generateButton.isEnabled = false
            sendInstruction(instruction: inst)
        }
    }
    
    /**
     Re-enable the transmit button on bluetooth disconnecting
     */
    override func onDisconnect() {
        super.onDisconnect()
        
        self.generateButton.isEnabled = true
    }
    
    /**
     Compile the next instruction to send
     */
    override func compileNextInstruction(fromAck: Bool) -> Instruction? {
        /**
         If we are compiling the instruction because an acknowledge characteristic, set our next target
         */
        if fromAck {
            setNextTarget()
        }
        
        guard let waypoint = arView.currentWayPoint else {
            // If there are no more waypoints, re-enable the transmit button
            self.generateButton.isEnabled = true
            return nil
        }
 
        // If you have a checkpoint or know the robots location, then use it
        guard let trackedObject: TrackedObject = arView.robot ?? arView.lastCheckpoint else {
            let alertView = UIAlertController(title: "Error", message: "Need to localize robot", preferredStyle: .alert)
            alertView.addAction(UIAlertAction(title: "Ok", style: .cancel, handler: nil))
            self.present(alertView, animated: true, completion: nil)
            
            self.generateButton.isEnabled = true
            return nil
        }
        
        print("Using last known location from \(type(of: trackedObject))")
        // Compile the instruction
        let inst = Instruction(distance: trackedObject.distanceTo(waypoint), angle: trackedObject.angleTo(waypoint), waypointNumber: waypoint.number)
        
        // We assume the robot will achieve the checkpoint when if is not being tracked
        arView.lastCheckpoint = Checkpoint(reference: waypoint, orientation: (arView.lastCheckpoint?.orientation ?? 0.0) + inst.angle)
        print("Robot orientation \(arView.lastCheckpoint?.orientation)")

        return inst
    }
    
    /**
     Helper function which determines whether or not the robot has reached the waypoint (within 10 cm) and sets the target to be the next waypoint if it has
     */
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

/**
 Implementations of the AR Session Delegate for Image Tracking
 */
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
