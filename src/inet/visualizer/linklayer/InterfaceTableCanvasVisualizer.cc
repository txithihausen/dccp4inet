//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/visualizer/linklayer/InterfaceTableCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(InterfaceTableCanvasVisualizer);

InterfaceTableCanvasVisualizer::InterfaceCanvasVisualization::InterfaceCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, BoxedLabelFigure *figure, int networkNodeId, int interfaceId) :
    InterfaceVisualization(networkNodeId, interfaceId),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

InterfaceTableCanvasVisualizer::InterfaceCanvasVisualization::~InterfaceCanvasVisualization()
{
    delete figure;
}

void InterfaceTableCanvasVisualizer::initialize(int stage)
{
    InterfaceTableVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer = getModuleFromPar<NetworkNodeCanvasVisualizer>(par("networkNodeVisualizerModule"), this);
    }
}

InterfaceTableVisualizerBase::InterfaceVisualization *InterfaceTableCanvasVisualizer::createInterfaceVisualization(cModule *networkNode, InterfaceEntry *interfaceEntry)
{
    auto figure = new BoxedLabelFigure("networkInterface");
    figure->setTags("network_interface");
    figure->setTooltip("This label represents a network interface in a network node");
    figure->setAssociatedObject(interfaceEntry);
    figure->setZIndex(zIndex);
    figure->setFont(font);
    figure->setText(getVisualizationText(interfaceEntry).c_str());
    figure->setLabelColor(textColor);
    figure->setBackgroundColor(backgroundColor);
    figure->setOpacity(opacity);
    auto networkNodeVisualization = networkNodeVisualizer->getNeworkNodeVisualization(networkNode);
    return new InterfaceCanvasVisualization(networkNodeVisualization, figure, networkNode->getId(), interfaceEntry->getInterfaceId());
}

cGate *InterfaceTableCanvasVisualizer::getOutputGate(const InterfaceVisualization *interfaceVisualization)
{
    L3AddressResolver addressResolver;
    auto networkNode = getSimulation()->getModule(interfaceVisualization->networkNodeId);
    if (networkNode == nullptr)
        return nullptr;
    auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
    if (interfaceTable == nullptr)
        return nullptr;
    auto interfaceEntry = interfaceTable->getInterfaceById(interfaceVisualization->interfaceId);
    if (interfaceEntry == nullptr || interfaceEntry->getNodeOutputGateId() == -1)
        return nullptr;
    cGate *outputGate = networkNode->gate(interfaceEntry->getNodeOutputGateId());
    if (outputGate == nullptr || outputGate->getChannel() == nullptr)
        return nullptr;
    else
        return outputGate;
}

void InterfaceTableCanvasVisualizer::addInterfaceVisualization(const InterfaceVisualization *interfaceVisualization)
{
    InterfaceTableVisualizerBase::addInterfaceVisualization(interfaceVisualization);
    auto interfaceCanvasVisualization = static_cast<const InterfaceCanvasVisualization *>(interfaceVisualization);
    auto gate = getOutputGate(interfaceVisualization);
    if (gate != nullptr) {
        cDisplayString& displayString = gate->getDisplayString();
        displayString.setTagArg("t", 0, interfaceCanvasVisualization->figure->getText());
        displayString.setTagArg("t", 1, "l");
    }
    else
        interfaceCanvasVisualization->networkNodeVisualization->addAnnotation(interfaceCanvasVisualization->figure, interfaceCanvasVisualization->figure->getBounds().getSize());
}

void InterfaceTableCanvasVisualizer::removeInterfaceVisualization(const InterfaceVisualization *interfaceVisualization)
{
    InterfaceTableVisualizerBase::removeInterfaceVisualization(interfaceVisualization);
    auto interfaceCanvasVisualization = static_cast<const InterfaceCanvasVisualization *>(interfaceVisualization);
    auto gate = getOutputGate(interfaceVisualization);
    if (gate != nullptr) {
        cDisplayString& displayString = gate->getDisplayString();
        displayString.setTagArg("t", 0, "");
    }
    else
        interfaceCanvasVisualization->networkNodeVisualization->removeAnnotation(interfaceCanvasVisualization->figure);
}

void InterfaceTableCanvasVisualizer::refreshInterfaceVisualization(const InterfaceVisualization *interfaceVisualization, const InterfaceEntry *interfaceEntry)
{
    auto interfaceCanvasVisualization = static_cast<const InterfaceCanvasVisualization *>(interfaceVisualization);
    auto figure = interfaceCanvasVisualization->figure;
    figure->setText(getVisualizationText(interfaceEntry).c_str());
    interfaceCanvasVisualization->networkNodeVisualization->setAnnotationSize(figure, figure->getBounds().getSize());
}

} // namespace visualizer

} // namespace inet
