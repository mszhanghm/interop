//
// Created by dev on 4/29/16.
//
#include <gtest/gtest.h>
#include "interop/model/plot/filter_options.h"
#include "interop/logic/plot/plot_by_cycle.h"
#include "interop/logic/plot/plot_by_lane.h"
#include "interop/logic/plot/plot_qscore_histogram.h"
#include "interop/logic/plot/plot_qscore_heatmap.h"
#include "interop/logic/plot/plot_flowcell_map.h"
#include "interop/logic/plot/plot_sample_qc.h"
#include "src/tests/interop/metrics/inc/extraction_metrics_test.h"
#include "src/tests/interop/metrics/inc/tile_metrics_test.h"
#include "src/tests/interop/metrics/inc/q_metrics_test.h"
#include "src/tests/interop/metrics/inc/index_metrics_test.h"

using namespace illumina::interop;

TEST(plot_logic, intensity_by_cycle)
{
    const float tol = 1e-3f;
    model::metrics::run_metrics metrics;
    model::plot::filter_options options(constants::FourDigit);
    std::vector<model::run::read_info> reads(2);
    reads[0] = model::run::read_info(1, 1, 26);
    reads[1] = model::run::read_info(2, 27, 76);
    metrics.run_info(model::run::info(
            "",
            "",
            1,
            model::run::flowcell_layout(2, 2, 2, 16),
            std::vector<std::string>(),
            model::run::image_dimensions(),
            reads
    ));
    metrics.legacy_channel_update(constants::HiSeq);

    std::istringstream iss(unittest::extraction_v2::binary_data());
    io::read_metrics(iss, metrics.extraction_metric_set());

    model::plot::plot_data<model::plot::candle_stick_point> data;
    logic::plot::plot_by_cycle(metrics, constants::Intensity, options, data);
    ASSERT_EQ(data.size(), 4u);
    EXPECT_EQ(data.x_axis().label(), "Cycle");
    EXPECT_EQ(data.y_axis().label(), "Intensity");
    EXPECT_EQ(data.title(), "All Lanes All Channels All Surfaces");
    EXPECT_NEAR(data.x_axis().min(), 0.0f, tol);
    EXPECT_NEAR(data.y_axis().min(), 0.0f, tol);
    EXPECT_NEAR(data.x_axis().max(), 3.0f, tol);
    EXPECT_NEAR(data.y_axis().max(), 353.1f, tol);
    for(size_t channel=0;channel<4u;++channel)
    {
        for(size_t i=0;i<data[channel].size();++i)
        {
            EXPECT_EQ(data[channel][i].x(), i+1);
            // Could check y, but ...
        }
    }
}


TEST(plot_logic, pf_clusters_by_lane)
{
    const float tol = 1e-3f;
    model::metrics::run_metrics metrics;
    model::plot::filter_options options(constants::FourDigit);
    std::vector<model::run::read_info> reads(2);
    reads[0] = model::run::read_info(1, 1, 26);
    reads[1] = model::run::read_info(2, 27, 76);
    metrics.run_info(model::run::info(
            "",
            "",
            1,
            model::run::flowcell_layout(2, 2, 2, 16),
            std::vector<std::string>(),
            model::run::image_dimensions(),
            reads
    ));
    metrics.legacy_channel_update(constants::HiSeq);

    std::istringstream iss(unittest::tile_v2::binary_data());
    io::read_metrics(iss, metrics.tile_metric_set());

    model::plot::plot_data<model::plot::candle_stick_point> data;
    logic::plot::plot_by_lane(metrics, constants::ClusterCountPF, options, data);
    ASSERT_EQ(data.size(), 1u);
    EXPECT_EQ(data.title(), "");
    EXPECT_EQ(data.x_axis().label(), "Lane");
    EXPECT_EQ(data.y_axis().label(), "Clusters PF");
    EXPECT_NEAR(data.x_axis().min(), 0.0f, tol);
    EXPECT_NEAR(data.y_axis().min(), 0.0f, tol);
    EXPECT_NEAR(data.x_axis().max(), 8.0f, tol);
    EXPECT_NEAR(data.y_axis().max(), 4.259528636932373f, tol);

}

TEST(plot_logic, q_score_histogram)
{
    const float tol = 1e-3f;
    model::metrics::run_metrics metrics;
    model::plot::filter_options options(constants::FourDigit);
    std::vector<model::run::read_info> reads(2);
    reads[0] = model::run::read_info(1, 1, 26);
    reads[1] = model::run::read_info(2, 27, 76);
    metrics.run_info(model::run::info(
            "",
            "",
            1,
            model::run::flowcell_layout(2, 2, 2, 16),
            std::vector<std::string>(),
            model::run::image_dimensions(),
            reads
    ));
    metrics.legacy_channel_update(constants::HiSeq);
    metrics.set_naming_method(constants::FourDigit);

    std::istringstream iss(unittest::q_v6::binary_data());
    io::read_metrics(iss, metrics.q_metric_set());
    metrics.finalize_after_load();

    model::plot::plot_data<model::plot::bar_point> data;
    logic::plot::plot_qscore_histogram(metrics, options, data);
    ASSERT_EQ(data.size(), 1u);
    EXPECT_EQ(data.title(), "All Lanes");
    EXPECT_EQ(data.x_axis().label(), "Q Score");
    EXPECT_EQ(data.y_axis().label(), "Total (million)");
    EXPECT_NEAR(data.x_axis().min(), 1.0f, tol);
    EXPECT_NEAR(data.y_axis().min(), 0.0f, tol);
    EXPECT_NEAR(data.x_axis().max(), 45.100002288818359f, tol);
    EXPECT_NEAR(data.y_axis().max(), 9.4780035018920898f, tol);
}

TEST(plot_logic, q_score_heatmap)
{
    const float tol = 1e-3f;
    model::metrics::run_metrics metrics;
    model::plot::filter_options options(constants::FourDigit);
    std::vector<model::run::read_info> reads(2);
    reads[0] = model::run::read_info(1, 1, 26);
    reads[1] = model::run::read_info(2, 27, 76);
    metrics.run_info(model::run::info(
            "",
            "",
            1,
            model::run::flowcell_layout(2, 2, 2, 16),
            std::vector<std::string>(),
            model::run::image_dimensions(),
            reads
    ));
    metrics.legacy_channel_update(constants::HiSeq);
    metrics.set_naming_method(constants::FourDigit);

    std::istringstream iss(unittest::q_v6::binary_data());
    io::read_metrics(iss, metrics.q_metric_set());
    metrics.finalize_after_load();

    model::plot::heatmap_data data;
    logic::plot::plot_qscore_heatmap(metrics, options, data);
    ASSERT_EQ(data.row_count(), 4u);
    EXPECT_EQ(data.title(), "All Lanes");
    EXPECT_EQ(data.x_axis().label(), "Cycle");
    EXPECT_EQ(data.y_axis().label(), "Q Score");
    EXPECT_NEAR(data.x_axis().min(), 0.0f, tol);
    EXPECT_NEAR(data.y_axis().min(), 0.0f, tol);
    EXPECT_NEAR(data.x_axis().max(), 4.0f, tol);
    EXPECT_NEAR(data.y_axis().max(), 40.0f, tol);
}

TEST(plot_logic, flowcell_map)
{
    const model::plot::filter_options::id_t ALL_IDS = model::plot::filter_options::ALL_IDS;
    const float tol = 1e-3f;
    model::metrics::run_metrics metrics;
    std::vector<model::run::read_info> reads(2);
    reads[0] = model::run::read_info(1, 1, 26);
    reads[1] = model::run::read_info(2, 27, 76);
    metrics.run_info(model::run::info(
            "",
            "",
            1,
            model::run::flowcell_layout(8, 2, 2, 36, 1, 1, std::vector<std::string>(), constants::FourDigit),
            std::vector<std::string>(),
            model::run::image_dimensions(),
            reads
    ));
    model::plot::filter_options options(constants::FourDigit, ALL_IDS, 0, constants::A, ALL_IDS, 1, 1);
    metrics.legacy_channel_update(constants::HiSeq);

    std::istringstream iss(unittest::extraction_v2::binary_data());
    io::read_metrics(iss, metrics.extraction_metric_set());
    ASSERT_GT(metrics.extraction_metric_set().size(), 0u);

    model::plot::flowcell_data data;
    logic::plot::plot_flowcell_map(metrics, constants::Intensity, options, data);
    ASSERT_EQ(data.row_count(), 8u);
    EXPECT_EQ(data.title(), "Intensity");
    EXPECT_EQ(data.saxis().label(), "Intensity");
    EXPECT_NEAR(data.saxis().min(), 302.0f, tol);
    EXPECT_NEAR(data.saxis().max(), 349.0f, tol);
}

TEST(plot_logic, sample_qc)
{
    const model::plot::filter_options::id_t ALL_IDS = model::plot::filter_options::ALL_IDS;
    const float tol = 1e-3f;
    model::metrics::run_metrics metrics;
    std::vector<model::run::read_info> reads(2);
    reads[0] = model::run::read_info(1, 1, 26);
    reads[1] = model::run::read_info(2, 27, 76);
    metrics.run_info(model::run::info(
            "",
            "",
            1,
            model::run::flowcell_layout(8, 2, 2, 36, 1, 1, std::vector<std::string>(), constants::FourDigit),
            std::vector<std::string>(),
            model::run::image_dimensions(),
            reads
    ));
    model::plot::filter_options options(constants::FourDigit, ALL_IDS, 0, constants::A, ALL_IDS, 1, 1);
    metrics.legacy_channel_update(constants::HiSeq);

    std::istringstream iss(unittest::index_v1::binary_data());
    io::read_metrics(iss, metrics.index_metric_set());
    std::istringstream iss2(unittest::tile_v2::binary_data());
    io::read_metrics(iss2, metrics.tile_metric_set());
    ASSERT_GT(metrics.index_metric_set().size(), 0u);

    model::plot::plot_data<model::plot::bar_point> data;
    logic::plot::plot_sample_qc(metrics, 1, data);
    ASSERT_EQ(data.size(), 1u);
    EXPECT_EQ(data[0].title(), "% reads");
    EXPECT_EQ(data.x_axis().label(), "Index Number");
    EXPECT_EQ(data.y_axis().label(), "% Reads Identified (PF)");
    EXPECT_NEAR(data.x_axis().min(), 0.0f, tol);
    EXPECT_NEAR(data.y_axis().min(), 0.0f, tol);
    EXPECT_NEAR(data.x_axis().max(), 0, tol);
    EXPECT_NEAR(data.y_axis().max(), 5, tol);
}
