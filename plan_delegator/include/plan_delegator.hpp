/*
 * Copyright (C) 2019 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#ifndef PLAN_DELEGATOR_INCLUDE_PLAN_DELEGATOR_HPP_
#define PLAN_DELEGATOR_INCLUDE_PLAN_DELEGATOR_HPP_

#include <unordered_map>
#include <math.h>
#include <ros/ros.h>
#include <cav_msgs/ManeuverPlan.h>
#include <cav_srvs/PlanTrajectory.h>
#include <carma_utils/CARMAUtils.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>

// TODO Replace this Macro if possible
/**
 * \brief Macro definition to enable easier access to fields shared across the maneuver typees
 * \param mvr The maneuver object to invoke the accessors on
 * \param property The name of the field to access on the specific maneuver types. Must be shared by all extant maneuver types
 * \return Expands to an expression (in the form of chained ternary operators) that evalutes to the desired field
 */
#define GET_MANEUVER_PROPERTY(mvr, property)\
        (((mvr).type == cav_msgs::Maneuver::INTERSECTION_TRANSIT_LEFT_TURN ? (mvr).intersection_transit_left_turn_maneuver.property :\
            ((mvr).type == cav_msgs::Maneuver::INTERSECTION_TRANSIT_RIGHT_TURN ? (mvr).intersection_transit_right_turn_maneuver.property :\
                ((mvr).type == cav_msgs::Maneuver::INTERSECTION_TRANSIT_STRAIGHT ? (mvr).intersection_transit_straight_maneuver.property :\
                    ((mvr).type == cav_msgs::Maneuver::LANE_CHANGE ? (mvr).lane_change_maneuver.property :\
                        ((mvr).type == cav_msgs::Maneuver::LANE_FOLLOWING ? (mvr).lane_following_maneuver.property :\
                            throw new std::invalid_argument("GET_MANEUVER_PROPERTY (property) called on maneuver with invalid type id")))))))

namespace plan_delegator
{
    class PlanDelegator
    {
        public:

            // constants definition
            static const constexpr double MILLISECOND_TO_SECOND = 0.001;

            PlanDelegator();

            /**
             * \brief Initialize the plan delegator
             */
            void init();

            /**
             * \brief Run the main thread of plan delegator
             */
            void run();

        private:

            // nodehandle and private nodehandle
            ros::NodeHandle nh_;
            ros::NodeHandle pnh_;

            // ROS subscribers and publishers
            ros::Publisher traj_pub_;
            ros::Subscriber plan_sub_;
            ros::Subscriber pose_sub_;
            ros::Subscriber twist_sub_;

            // ROS params
            std::string planning_topic_prefix_;
            std::string planning_topic_suffix_;
            double spin_rate_, max_trajectory_duration_;

            // map to store service clients
            std::unordered_map<std::string, ros::ServiceClient> trajectory_planners_;
            // local storage of incoming messages
            cav_msgs::ManeuverPlan latest_maneuver_plan_;
            geometry_msgs::PoseStamped latest_pose_;
            geometry_msgs::TwistStamped latest_twist_;

            /**
             * \brief Callback function of maneuver plan subscriber
             */
            void ManeuverPlanCallback(const cav_msgs::ManeuverPlanConstPtr& plan);

            /**
             * \brief Callback function of node spin
             * \return if callback function runs successfully
             */
            bool SpinCallback();

            /**
             * \brief Example if a maneuver end time has passed current system time
             * \return if input maneuver is expires
             */
            bool IsManeuverExpired(const cav_msgs::Maneuver& maneuver) const noexcept;

            /**
             * \brief Example if a maneuver plan contains at least one maneuver
             * \return if input maneuver plan is valid
             */
            bool IsManeuverPlanValid(const cav_msgs::ManeuverPlanConstPtr& maneuver_plan) const noexcept;

            /**
             * \brief Example if a trajectory plan contains at least two trajectory points
             * \return if input trajectory plan is valid
             */
            bool IsTrajectoryValid(const cav_msgs::TrajectoryPlan& trajectory_plan) const noexcept;

            /**
             * \brief Example if a trajectory plan is longer than configured time thresheld
             * \return if input trajectory plan is long enough
             */
            bool IsTrajectoryLongEnough(const cav_msgs::TrajectoryPlan& plan) const noexcept;

            /**
             * \brief Generate new PlanTrajecory service request because current planning progress
             * \return a PlanTrajectory object which is ready to be used in the following service call
             */
            cav_srvs::PlanTrajectory ComposePlanTrajectoryRequest(const cav_msgs::TrajectoryPlan& latest_trajectory_plan) const;

            /**
             * \brief Plan trajectory based on latest maneuver plan via ROS service call to plugins
             * \return a TrajectoryPlan object which contains PlanTrajectory response from plugins
             */
            cav_msgs::TrajectoryPlan PlanTrajectory();

            /**
             * \brief Get PlanTrajectory service client by plugin name and
             * create new PlanTrajectory service client if specified name does not exist
             * \return a ServiceClient object which corresponse to the target planner
             */
            ros::ServiceClient& GetPlannerClientByName(const std::string& planner_name);
    };
}
#endif // PLAN_DELEGATOR_INCLUDE_PLAN_DELEGATOR_HPP_